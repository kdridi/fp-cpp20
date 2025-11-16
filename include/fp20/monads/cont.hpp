// ============================================================================
// FP++20: Cont<R, A> Monad Implementation (Continuation Monad)
// ============================================================================
// The Continuation monad represents computations in continuation-passing style.
// It's the MOST POWERFUL monad - it can encode all other monads and provides
// direct control over program flow through first-class continuations.
//
// ============================================================================
// ACADEMIC REFERENCES & FOUNDATIONAL LITERATURE
// ============================================================================
//
// Haskell Implementation:
// - Reference: [Control.Monad.Cont] - The mtl package
//   https://hackage.haskell.org/package/mtl/docs/Control-Monad-Cont.html
//   The canonical Continuation monad with callCC primitive
//
// - Reference: [Continuation Monad Documentation]
//   https://wiki.haskell.org/Continuation
//   Comprehensive guide to continuation-passing style
//
// Category Theory Foundation:
// - Reference: [Monads for Functional Programming] - Philip Wadler (1995)
//   Advanced Functional Programming, Springer LNCS 925
//   Section on Continuation monad and callCC
//
// - Reference: [Notions of Computation and Monads] - Eugenio Moggi (1991)
//   Information and Computation 93(1)
//   Foundational paper on computational effects via monads
//
// Control Flow & CPS:
// - Reference: [Continuations: A Mathematical Semantics for Handling Full Jumps]
//   Christopher Strachey & Christopher P. Wadsworth (1974)
//   Original formalization of continuations
//
// - Reference: [Representing Control in the Presence of First-Class Continuations]
//   Olivier Danvy & Andrzej Filinski (1990)
//   POPL '90 - formalization of callCC operator
//
// - Reference: [A Monadic Framework for Delimited Continuations]
//   R. Kent Dybvig, Simon Peyton Jones, Amr Sabry (2007)
//   JFP 17(6) - Delimited continuations and control operators
//
// Double-Negation Translation:
// - Reference: [On the Continuation of Continuations]
//   Cont r a ≅ ((a → r) → r) ≅ ¬¬a in classical logic
//   Continuation monad corresponds to double-negation in logic
//
// Category Theory Perspective:
// - Cont r a ≅ (a → r) → r (CPS transformation)
// - Functor instance: pre-composition on continuations
// - Monad instance: Kleisli composition in CPS
// - callCC: call-with-current-continuation (escape operator)
//
// ============================================================================
// CONTINUATION MONAD OVERVIEW
// ============================================================================
//
// KEY INSIGHT: Cont<R, A> ≅ ((A → R) → R)
//
// The Continuation monad represents computations in CPS (continuation-passing style):
// 1. Instead of returning a value, pass it to a continuation
// 2. The continuation is a function that consumes the value
// 3. Enables non-local control flow (early exit, exceptions, backtracking)
//
// CORE OPERATIONS:
// - runCont: Execute the continuation with a given final continuation
// - callCC: Call-with-current-continuation - captures control flow
//
// MONADIC STRUCTURE:
// - Functor: Transform value before passing to continuation
// - Applicative: Thread continuation through function application
// - Monad: Compose continuations sequentially
//
// ============================================================================
// MONAD LAWS FOR CONT
// ============================================================================
//
// Standard Monad Laws:
// 1. Left Identity: return a >>= f ≡ f a
//    pure(x) >>= f  ==  f(x)
//
// 2. Right Identity: m >>= return ≡ m
//    m >>= pure  ==  m
//
// 3. Associativity: (m >>= f) >>= g ≡ m >>= (λx -> f x >>= g)
//    bind(bind(m, f), g)  ==  bind(m, [](auto x) { return bind(f(x), g); })
//
// Continuation-Specific Laws:
// 4. callCC escape: callCC (\k -> k v >>= m) ≡ return v
//    Using escape continuation short-circuits computation
//
// 5. callCC ignore: callCC (\_ -> m) ≡ m  (if k not used)
//    Not using escape is same as normal computation
//
// ============================================================================

#pragma once

#include <functional>
#include <type_traits>
#include <utility>
#include <concepts>

namespace fp20 {

    // ========================================================================
    // UNIT TYPE FOR VOID-LIKE CONTINUATIONS
    // ========================================================================
    // callCC often needs a "void-like" type for escape continuations.
    // We use Unit as a monoidal unit type (like () in Haskell).
    struct Unit {
        constexpr bool operator==(const Unit&) const noexcept = default;
    };

    // ========================================================================
    // CONT TYPE - Continuation-Passing Style Computation
    // ========================================================================
    // Cont<R, A> wraps a function ((A → R) → R)
    // This is the CPS transformation of a computation producing A
    //
    // R: Final result type (what the continuation ultimately produces)
    // A: Intermediate value type (what this computation produces)
    template<typename R, typename A>
    class Cont {
    public:
        using result_type = R;
        using value_type = A;

    private:
        // The continuation computation: ((A → R) → R)
        // Takes a continuation (A → R) and produces final result R
        std::function<R(std::function<R(A)>)> computation_;

    public:
        // Construct Cont from a CPS function
        explicit Cont(std::function<R(std::function<R(A)>)> f)
            : computation_(std::move(f)) {}

        // Template constructor for any callable
        template<typename F>
            requires std::invocable<F, std::function<R(A)>> &&
                     std::same_as<std::invoke_result_t<F, std::function<R(A)>>, R>
        explicit Cont(F&& f)
            : computation_(std::forward<F>(f)) {}

        // Copy/move semantics (defaulted)
        Cont(const Cont&) = default;
        Cont(Cont&&) noexcept = default;
        Cont& operator=(const Cont&) = default;
        Cont& operator=(Cont&&) noexcept = default;

        // Run the continuation with the given final continuation
        // This extracts the value by providing what to do with it
        R runCont(std::function<R(A)> k) const {
            return computation_(k);
        }
    };

    // ========================================================================
    // FUNCTOR INSTANCE - Pre-Composition
    // ========================================================================
    // fmap :: (a → b) → Cont r a → Cont r b
    // fmap f m = Cont (\k -> runCont m (k . f))
    //
    // Transforms the intermediate value before passing to continuation.
    // Mathematically: fmap f = (. f)  (pre-composition with continuation)
    //
    // Time complexity: O(1) to construct, O(f + m) to execute
    // Space complexity: O(1)
    template<typename Func, typename R, typename A>
    auto fmap(Func&& f, const Cont<R, A>& cont) {
        using B = std::decay_t<decltype(f(std::declval<A>()))>;

        return Cont<R, B>([cont, f = std::forward<Func>(f)](std::function<R(B)> k) -> R {
            // Run cont with continuation that first applies f, then k
            return cont.runCont([f, k](A a) -> R {
                return k(f(a));
            });
        });
    }

    // ========================================================================
    // APPLICATIVE INSTANCE - Continuation Threading
    // ========================================================================

    // pure :: a → Cont r a
    // pure x = Cont (\k -> k x)
    //
    // Creates a Cont that immediately passes the value to its continuation.
    // This is the minimal/trivial continuation computation.
    //
    // Time complexity: O(1)
    // Space complexity: O(1)
    template<typename R, typename A>
    Cont<R, A> pure(A value) {
        return Cont<R, A>([v = std::move(value)](std::function<R(A)> k) -> R {
            return k(v);
        });
    }

    // Explicit two-template-parameter version for clarity
    template<template<typename...> typename M, typename R, typename A>
        requires std::same_as<M<R, A>, Cont<R, A>>
    Cont<R, A> pure(A value) {
        return pure<R, A>(std::move(value));
    }

    // apply :: Cont r (a → b) → Cont r a → Cont r b
    // apply cf cx = Cont (\k -> runCont cf (\f -> runCont cx (\x -> k (f x))))
    //
    // Applies a function in Cont context to a value in Cont context.
    // Both computations are threaded through the continuation.
    //
    // Time complexity: O(1) to construct, O(cf + cx) to execute
    // Space complexity: O(1)
    template<typename R, typename Func, typename A>
    auto apply(const Cont<R, Func>& cf, const Cont<R, A>& cx) {
        using B = std::decay_t<decltype(std::declval<Func>()(std::declval<A>()))>;

        return Cont<R, B>([cf, cx](std::function<R(B)> k) -> R {
            return cf.runCont([cx, k](Func f) -> R {
                return cx.runCont([f, k](A x) -> R {
                    return k(f(x));
                });
            });
        });
    }

    // ========================================================================
    // MONAD INSTANCE - Continuation Composition
    // ========================================================================

    // return_ :: a → Cont r a
    // Alias for pure (monadic return).
    template<template<typename...> typename M, typename R, typename A>
        requires std::same_as<M<R, A>, Cont<R, A>>
    Cont<R, A> return_(A value) {
        return pure<R, A>(std::move(value));
    }

    // bind :: Cont r a → (a → Cont r b) → Cont r b
    // bind m k = Cont (\c -> runCont m (\a -> runCont (k a) c))
    //
    // Sequences continuation computations.
    // The continuation k can use the value from m to determine what to compute next.
    //
    // KEY INSIGHT: Both m and k(a) receive the SAME final continuation c
    //
    // Time complexity: O(1) to construct, O(m + k) to execute
    // Space complexity: O(1)
    template<typename R, typename A, typename Func>
    auto bind(const Cont<R, A>& m, Func&& k)
    {
        using ContB = std::decay_t<decltype(k(std::declval<A>()))>;
        using B = typename ContB::value_type;

        return Cont<R, B>([m, k_copy = std::forward<Func>(k)](std::function<R(B)> c) mutable -> R {
            // Run m with a continuation that:
            // 1. Takes the value a from m
            // 2. Passes it to k to get a new Cont
            // 3. Runs that Cont with the final continuation c
            return m.runCont([k_copy, c](A a) mutable -> R {
                return k_copy(a).runCont(c);
            });
        });
    }

    // ========================================================================
    // CONTINUATION PRIMITIVES - Control Flow Operations
    // ========================================================================

    // callCC :: ((a → Cont r a) → Cont r a) → Cont r a
    // Call-with-current-continuation
    //
    // Captures the current continuation and makes it available as an escape function.
    // If the escape function is called, the rest of the computation is skipped.
    //
    // The escape function has type (a → Cont r a).
    // When invoked, it IGNORES any subsequent bind operations and immediately
    // returns to the captured continuation with the given value.
    //
    // This is the fundamental control-flow primitive: early exit, exceptions, etc.
    //
    // Time complexity: O(1) to construct, O(f) to execute
    // Space complexity: O(1)
    template<typename R, typename A, typename Func>
        requires std::invocable<Func, std::function<Cont<R, A>(A)>>
    auto callCC(Func&& f) {
        using ResultCont = std::invoke_result_t<Func, std::function<Cont<R, A>(A)>>;
        static_assert(std::same_as<ResultCont, Cont<R, A>>,
            "callCC function must return Cont<R, A>");

        return Cont<R, A>([f = std::forward<Func>(f)](std::function<R(A)> k) -> R {
            // Create the escape continuation
            // When called with value a, it creates a Cont that IGNORES its continuation
            // and immediately calls the captured continuation k with the value
            auto escape = [k](A a) -> Cont<R, A> {
                return Cont<R, A>([k, a](std::function<R(A)>) -> R {
                    // Ignore the continuation parameter, use captured k
                    return k(a);
                });
            };

            // Call f with the escape continuation, then run result with k
            return f(escape).runCont(k);
        });
    }

    // ========================================================================
    // HELPER FUNCTIONS
    // ========================================================================

    // evalCont :: Cont r r → r
    // Run a continuation computation with the identity continuation.
    // This is the simplest way to extract a value when R = A.
    //
    // For Cont<R, R>, we can just use identity as the continuation.
    template<typename R>
    R evalCont(const Cont<R, R>& cont) {
        return cont.runCont([](R r) -> R { return r; });
    }

    // mapCont :: (r → r) → Cont r a → Cont r a
    // Transform the final result of a continuation computation.
    // This modifies what the continuation ultimately produces.
    template<typename R, typename A, typename Func>
        requires std::invocable<Func, R> && std::same_as<std::invoke_result_t<Func, R>, R>
    Cont<R, A> mapCont(Func&& f, const Cont<R, A>& cont) {
        return Cont<R, A>([cont, f = std::forward<Func>(f)](std::function<R(A)> k) -> R {
            return f(cont.runCont(k));
        });
    }

    // withCont :: ((b → r) → (a → r)) → Cont r a → Cont r b
    // Modify the continuation before running the computation.
    // This is the most general continuation transformation.
    template<typename R, typename A, typename B, typename Func>
        requires std::invocable<Func, std::function<R(A)>> &&
                 std::same_as<std::invoke_result_t<Func, std::function<R(A)>>, std::function<R(B)>>
    Cont<R, B> withCont(Func&& f, const Cont<R, A>& cont) {
        return Cont<R, B>([cont, f = std::forward<Func>(f)](std::function<R(B)> k) -> R {
            return cont.runCont(f(k));
        });
    }

    // ========================================================================
    // USAGE PATTERNS AND EXAMPLES
    // ========================================================================
    //
    // 1. BASIC CONTINUATION:
    // ```cpp
    // auto c = pure<int>(42);
    // int result = c.runCont([](int x) { return x * 2; });  // 84
    // ```
    //
    // 2. EARLY RETURN WITH callCC:
    // ```cpp
    // auto computation = callCC<int, int>([](auto escape) {
    //     return bind(
    //         pure<int>(10),
    //         [escape](int x) {
    //             if (x > 5) return escape(999);  // Early exit!
    //             return pure<int>(x * 2);
    //         }
    //     );
    // });
    // int result = evalCont(computation);  // 999
    // ```
    //
    // 3. EXCEPTION-LIKE CONTROL:
    // ```cpp
    // auto safeDivide = [](int a, int b) {
    //     return callCC<int, int>([a, b](auto escape) {
    //         if (b == 0) return escape(-1);  // Error code
    //         return pure<int>(a / b);
    //         });
    // };
    // ```
    //
    // 4. CHAINING CONTINUATIONS:
    // ```cpp
    // auto c = bind(
    //     pure<int>(5),
    //     [](int x) {
    //         return bind(
    //             pure<int>(x * 2),
    //             [](int y) { return pure<int>(y + 3); }
    //         );
    //     }
    // );
    // int result = evalCont(c);  // 13
    // ```
    //
    // ========================================================================

} // namespace fp20
