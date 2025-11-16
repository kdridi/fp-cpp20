// ============================================================================
// FP++20: MaybeT<M, A> Monad Transformer Implementation
// ============================================================================
// MaybeT adds optional/nullable semantics to any monad M.
// MaybeT m a ≅ m (Maybe a) where Maybe a ≅ std::optional<a>
//
// This allows composing optional computations with other effects like IO, State, etc.
//
// ============================================================================
// ACADEMIC REFERENCES & FOUNDATIONAL LITERATURE
// ============================================================================
//
// Monad Transformers:
// - Reference: [Monad Transformers Step by Step] - Martin Grabmüller (2006)
//   https://page.mi.fu-berlin.de/scravy/realworldhaskell/materialien/monad-transformers-step-by-step.pdf
//   Comprehensive tutorial on building monad transformers
//
// - Reference: [Modular Monad Transformers] - Liang, Hudak, Jones (1995)
//   POPL '95: Proceedings of the 22nd ACM SIGPLAN-SIGACT symposium
//   Foundational paper on modular monad transformer design
//
// - Reference: [Haskell MaybeT] - Control.Monad.Trans.Maybe
//   https://hackage.haskell.org/package/transformers/docs/Control-Monad-Trans-Maybe.html
//   Standard Haskell implementation
//
// - Reference: [Monad Transformers and Modular Interpreters] - Espinosa (1995)
//   Shows practical applications of transformer stacks
//
// Category Theory:
// - Reference: [Functors, Applicatives, And Monads In Pictures]
//   https://adit.io/posts/2013-04-17-functors,_applicatives,_and_monads_in_pictures.html
//
// ============================================================================
// MAYBET MONAD TRANSFORMER OVERVIEW
// ============================================================================
//
// KEY INSIGHT: MaybeT<M, A> = M<std::optional<A>>
//
// MaybeT layers optional semantics on top of any monad M:
// - If the inner monad produces std::nullopt, computation short-circuits
// - If the inner monad produces std::optional<value>, computation continues
// - Combines failure handling with the effects of the base monad M
//
// STRUCTURE:
// - runMaybeT: Unwrap to get M<std::optional<A>>
// - lift: Lift M<A> to MaybeT<M, A> by wrapping in std::optional
// - Functor/Applicative/Monad instances compose both layers
//
// TYPICAL USAGE:
// - MaybeT<IO, User>: IO operations that may fail to find a user
// - MaybeT<State, Value>: Stateful computations that may fail
// - MaybeT<Reader, Config>: Configuration lookups that may not exist
//
// ============================================================================
// MONAD LAWS FOR MAYBET
// ============================================================================
//
// Standard Monad Laws (for MaybeT<M, A> where M is a Monad):
// 1. Left Identity: return a >>= f ≡ f a
//    bind(pure(x), f) == f(x)
//
// 2. Right Identity: m >>= return ≡ m
//    bind(m, pure) == m
//
// 3. Associativity: (m >>= f) >>= g ≡ m >>= (λx -> f x >>= g)
//    bind(bind(m, f), g) == bind(m, [](auto x) { return bind(f(x), g); })
//
// Transformer-Specific Laws:
// 4. lift . return ≡ return
//    lift(pure_m(x)) == pure_maybet(x)
//
// 5. lift (m >>= f) ≡ lift m >>= (lift . f)
//    Lifting preserves monadic structure
//
// ============================================================================

#pragma once

#include <optional>
#include <functional>
#include <type_traits>
#include <utility>
#include <concepts>

namespace fp20 {

    // Forward declarations for monad operations
    template<typename Func, typename M, typename A>
        requires requires(Func f, M m) { fmap(f, m); }
    auto fmap(Func&& f, M&& m);

    template<typename M, typename A, typename Func>
        requires requires(M m, Func f) { bind(m, f); }
    auto bind(M&& m, Func&& f);

    // ========================================================================
    // MAYBET TYPE - Optional Transformer
    // ========================================================================
    // MaybeT<M, A> represents a computation in monad M that may fail.
    // Internally: M<std::optional<A>>
    //
    // The value_ member is the inner monad containing an optional value.
    template<template<typename> class M, typename A>
    class MaybeT {
    public:
        using value_type = A;
        using inner_monad_type = M<std::optional<A>>;

    private:
        inner_monad_type value_;

    public:
        // ====================================================================
        // CONSTRUCTORS
        // ====================================================================

        // Construct MaybeT from the inner monad M<std::optional<A>>
        // Time complexity: O(1) + O(M constructor)
        // Space complexity: O(size of M<std::optional<A>>)
        explicit MaybeT(inner_monad_type value)
            : value_(std::move(value)) {}

        // Copy/move semantics (defaulted)
        MaybeT(const MaybeT&) = default;
        MaybeT(MaybeT&&) noexcept = default;
        MaybeT& operator=(const MaybeT&) = default;
        MaybeT& operator=(MaybeT&&) noexcept = default;

        // ====================================================================
        // UNWRAPPING
        // ====================================================================

        // runMaybeT :: MaybeT m a -> m (Maybe a)
        // Extract the inner monad computation.
        // Returns: M<std::optional<A>>
        //
        // Time complexity: O(copy/move M)
        // Space complexity: O(size of M)
        [[nodiscard]] inner_monad_type runMaybeT() const {
            return value_;
        }

        // Non-const version for move extraction
        [[nodiscard]] inner_monad_type runMaybeT() {
            return std::move(value_);
        }
    };

    // ========================================================================
    // SMART CONSTRUCTORS
    // ========================================================================

    // maybeT :: m (Maybe a) -> MaybeT m a
    // Wrap an M<std::optional<A>> into MaybeT.
    //
    // Time complexity: O(1)
    // Space complexity: O(1)
    template<template<typename> class M, typename A>
    MaybeT<M, A> maybeT(M<std::optional<A>> inner) {
        return MaybeT<M, A>(std::move(inner));
    }

    // ========================================================================
    // LIFT OPERATION - The Key Transformer Function
    // ========================================================================

    // lift :: Monad m => m a -> MaybeT m a
    // Lift a computation from the base monad M into MaybeT.
    // Wraps the result in std::optional (always succeeds).
    //
    // This is THE fundamental operation that makes MaybeT a transformer.
    //
    // Time complexity: O(fmap on M)
    // Space complexity: O(size of M)
    template<template<typename> class M, typename A>
        requires requires(M<A> ma) {
            { fmap([](A a) { return std::optional<A>(a); }, ma) } -> std::convertible_to<M<std::optional<A>>>;
        }
    MaybeT<M, A> lift(M<A> ma) {
        // Lift by mapping the inner value into std::optional
        auto wrapped = fmap([](A a) { return std::optional<A>(std::move(a)); }, std::move(ma));
        return MaybeT<M, A>(std::move(wrapped));
    }

    // ========================================================================
    // FUNCTOR INSTANCE - fmap over MaybeT
    // ========================================================================

    // fmap :: (a -> b) -> MaybeT m a -> MaybeT m b
    // Transform the value inside MaybeT, preserving both layers.
    //
    // Functor Laws:
    // 1. Identity: fmap(id, mt) ≡ mt
    // 2. Composition: fmap(g ∘ f, mt) ≡ fmap(g, fmap(f, mt))
    //
    // Time complexity: O(fmap on M)
    // Space complexity: O(result size)
    template<typename Func, template<typename> class M, typename A>
        requires std::invocable<Func, A>
    auto fmap(Func&& f, const MaybeT<M, A>& mt) {
        using B = std::invoke_result_t<Func, A>;

        // Map over the inner monad, transforming the optional value
        auto inner = mt.runMaybeT();
        auto mapped = fmap(
            [f = std::forward<Func>(f)](std::optional<A> opt) -> std::optional<B> {
                if (opt.has_value()) {
                    return std::optional<B>(f(std::move(*opt)));
                } else {
                    return std::nullopt;
                }
            },
            std::move(inner)
        );

        return MaybeT<M, B>(std::move(mapped));
    }

    // ========================================================================
    // APPLICATIVE INSTANCE
    // ========================================================================

    // pure :: a -> MaybeT m a
    // Lift a pure value into MaybeT.
    // Creates M<std::optional<A>> with the value wrapped.
    //
    // Requires: M has a pure operation
    //
    // Time complexity: O(pure on M)
    // Space complexity: O(size of result)
    template<template<typename> class MaybeT_T, template<typename> class M, typename A>
        requires std::same_as<MaybeT_T<A>, MaybeT<M, A>> &&
                 requires(A a) {
                     { pure<M, std::optional<A>>(std::optional<A>(a)) } -> std::convertible_to<M<std::optional<A>>>;
                 }
    MaybeT<M, A> pure(A value) {
        // Pure value wrapped in optional, then lifted into M
        return MaybeT<M, A>(pure<M, std::optional<A>>(std::optional<A>(std::move(value))));
    }

    // apply :: MaybeT m (a -> b) -> MaybeT m a -> MaybeT m b
    // Apply a function in MaybeT context to a value in MaybeT context.
    // Short-circuits if either is std::nullopt.
    //
    // Time complexity: O(bind on M)
    // Space complexity: O(result size)
    template<template<typename> class M, typename Func, typename A>
        requires std::invocable<Func, A>
    auto apply(const MaybeT<M, Func>& mtf, const MaybeT<M, A>& mta) {
        using B = std::invoke_result_t<Func, A>;

        // Bind over the function monad
        auto result = bind(
            mtf.runMaybeT(),
            [mta_inner = mta.runMaybeT()](std::optional<Func> opt_f) {
                if (!opt_f.has_value()) {
                    // Function is nullopt, return nullopt
                    return pure<M, std::optional<B>>(std::nullopt);
                }

                // Function exists, bind over the argument monad
                return bind(
                    mta_inner,
                    [f = std::move(*opt_f)](std::optional<A> opt_a) {
                        if (!opt_a.has_value()) {
                            // Argument is nullopt, return nullopt
                            return pure<M, std::optional<B>>(std::nullopt);
                        }

                        // Both exist, apply function
                        return pure<M, std::optional<B>>(
                            std::optional<B>(f(std::move(*opt_a)))
                        );
                    }
                );
            }
        );

        return MaybeT<M, B>(std::move(result));
    }

    // ========================================================================
    // MONAD INSTANCE
    // ========================================================================

    // bind :: MaybeT m a -> (a -> MaybeT m b) -> MaybeT m b
    // Sequences MaybeT computations with short-circuiting on std::nullopt.
    //
    // The key operation for MaybeT:
    // 1. Run the inner M computation to get std::optional<A>
    // 2. If nullopt, short-circuit and return M<nullopt>
    // 3. If value, apply continuation k to get MaybeT<M, B>
    // 4. Run that to get M<std::optional<B>>
    //
    // Monad Laws:
    // 1. Left Identity: bind(pure(a), k) ≡ k(a)
    // 2. Right Identity: bind(m, pure) ≡ m
    // 3. Associativity: bind(bind(m, f), g) ≡ bind(m, [](auto x) { return bind(f(x), g); })
    //
    // Time complexity: O(bind on M)
    // Space complexity: O(result size)
    template<template<typename> class M, typename A, typename Func>
        requires std::invocable<Func, A> &&
                 requires(Func f, A a) {
                     { f(a) } -> std::convertible_to<MaybeT<M, typename std::invoke_result_t<Func, A>::value_type>>;
                 }
    auto bind(const MaybeT<M, A>& m, Func&& k) {
        using MaybeTB = std::invoke_result_t<Func, A>;
        using B = typename MaybeTB::value_type;

        // Bind over the inner monad
        auto result = bind(
            m.runMaybeT(),
            [k = std::forward<Func>(k)](std::optional<A> opt) -> M<std::optional<B>> {
                if (!opt.has_value()) {
                    // Short-circuit: propagate failure
                    return pure<M, std::optional<B>>(std::nullopt);
                }

                // Success: apply continuation and unwrap
                MaybeTB mtb = k(std::move(*opt));
                return mtb.runMaybeT();
            }
        );

        return MaybeT<M, B>(std::move(result));
    }

    // return_ :: a -> MaybeT m a
    // Alias for pure (monadic return).
    template<template<typename> class MaybeT_T, template<typename> class M, typename A>
        requires std::same_as<MaybeT_T<A>, MaybeT<M, A>>
    MaybeT<M, A> return_(A value) {
        return pure<MaybeT_T, M, A>(std::move(value));
    }

    // ========================================================================
    // MAYBET-SPECIFIC OPERATIONS
    // ========================================================================

    // nothing :: MaybeT m a
    // Construct a MaybeT representing failure (std::nullopt).
    //
    // Time complexity: O(pure on M)
    // Space complexity: O(M)
    template<template<typename> class M, typename A>
        requires requires() {
            { pure<M, std::optional<A>>(std::nullopt) } -> std::convertible_to<M<std::optional<A>>>;
        }
    MaybeT<M, A> nothing() {
        return MaybeT<M, A>(pure<M, std::optional<A>>(std::nullopt));
    }

    // just :: a -> MaybeT m a
    // Construct a MaybeT with a successful value.
    // Equivalent to pure, but with clearer Maybe semantics.
    //
    // Time complexity: O(pure on M)
    // Space complexity: O(M + A)
    template<template<typename> class M, typename A>
        requires requires(A a) {
            { pure<M, std::optional<A>>(std::optional<A>(a)) } -> std::convertible_to<M<std::optional<A>>>;
        }
    MaybeT<M, A> just(A value) {
        return MaybeT<M, A>(pure<M, std::optional<A>>(std::optional<A>(std::move(value))));
    }

    // fromMaybe :: a -> MaybeT m a -> m a
    // Run MaybeT and provide a default value if it's std::nullopt.
    //
    // Time complexity: O(fmap on M)
    // Space complexity: O(M)
    template<template<typename> class M, typename A>
        requires requires(M<std::optional<A>> m, A default_val) {
            { fmap([default_val](std::optional<A> opt) { return opt.value_or(default_val); }, m) }
                -> std::convertible_to<M<A>>;
        }
    M<A> fromMaybe(A default_value, const MaybeT<M, A>& mt) {
        return fmap(
            [default_value = std::move(default_value)](std::optional<A> opt) {
                return opt.value_or(std::move(default_value));
            },
            mt.runMaybeT()
        );
    }

    // ========================================================================
    // HELPER FUNCTIONS
    // ========================================================================

    // liftMaybe :: Maybe a -> MaybeT m a
    // Lift a std::optional directly into MaybeT.
    //
    // Time complexity: O(pure on M)
    // Space complexity: O(M)
    template<template<typename> class M, typename A>
        requires requires(std::optional<A> opt) {
            { pure<M, std::optional<A>>(opt) } -> std::convertible_to<M<std::optional<A>>>;
        }
    MaybeT<M, A> liftMaybe(std::optional<A> opt) {
        return MaybeT<M, A>(pure<M, std::optional<A>>(std::move(opt)));
    }

    // mapMaybeT :: (m (Maybe a) -> n (Maybe b)) -> MaybeT m a -> MaybeT n b
    // Transform the inner monad computation.
    // Allows morphing between different base monads.
    //
    // Time complexity: O(f)
    // Space complexity: O(result)
    template<template<typename> class M, template<typename> class N, typename A, typename B, typename Func>
        requires std::invocable<Func, M<std::optional<A>>> &&
                 std::convertible_to<std::invoke_result_t<Func, M<std::optional<A>>>, N<std::optional<B>>>
    MaybeT<N, B> mapMaybeT(Func&& f, const MaybeT<M, A>& mt) {
        return MaybeT<N, B>(f(mt.runMaybeT()));
    }

    // ========================================================================
    // USAGE PATTERNS AND EXAMPLES
    // ========================================================================
    //
    // 1. BASIC MAYBET WITH IO:
    // ```cpp
    // auto findUser = [](int id) -> MaybeT<IO, User> {
    //     auto io_result = effect([id]() {
    //         return database_lookup(id);  // Returns std::optional<User>
    //     });
    //     return MaybeT<IO, User>(io_result);
    // };
    //
    // auto result = bind(findUser(42), [](User user) {
    //     return just<IO>(user.name);
    // });
    //
    // auto io_action = result.runMaybeT();
    // std::optional<std::string> name = io_action.unsafeRun();
    // ```
    //
    // 2. SHORT-CIRCUITING WITH MAYBET:
    // ```cpp
    // auto validate_age = [](User user) -> MaybeT<IO, User> {
    //     if (user.age >= 18) {
    //         return just<IO>(user);
    //     } else {
    //         return nothing<IO, User>();
    //     }
    // };
    //
    // auto pipeline = bind(findUser(42), validate_age);
    // // If user not found OR under 18, entire pipeline returns nullopt
    // ```
    //
    // 3. LIFTING BASE MONAD INTO MAYBET:
    // ```cpp
    // IO<std::string> log_message = putStrLn("Processing...");
    // MaybeT<IO, std::string> lifted = lift<IO>(std::move(log_message));
    // ```
    //
    // 4. STACKING TRANSFORMERS (MaybeT + StateT + IO):
    // ```cpp
    // // Type: MaybeT<StateT<AppState, IO>, User>
    // auto complex_operation = bind(
    //     lift<MaybeT<StateT<AppState, IO>>>(get_state()),
    //     [](AppState state) {
    //         return just<StateT<AppState, IO>>(state.current_user);
    //     }
    // );
    // ```
    //
    // ========================================================================

} // namespace fp20
