// ============================================================================
// FP++20: EitherT<M, L, R> Monad Transformer Implementation
// ============================================================================
// EitherT adds error handling with typed errors to any monad M.
// EitherT m l r ≅ m (Either l r)
//
// This allows composing error-handling computations with other effects like IO, State, etc.
//
// ============================================================================
// ACADEMIC REFERENCES & FOUNDATIONAL LITERATURE
// ============================================================================
//
// Monad Transformers:
// - Reference: [Monad Transformers Step by Step] - Martin Grabmüller (2006)
//   https://page.mi.fu-berlin.de/scravy/realworldhaskell/materialien/monad-transformers-step-by-step.pdf
//
// - Reference: [Haskell EitherT] - Control.Monad.Trans.Either
//   https://hackage.haskell.org/package/either/docs/Control-Monad-Trans-Either.html
//   Standard Haskell implementation
//
// - Reference: [Modular Monad Transformers] - Liang, Hudak, Jones (1995)
//   POPL '95: Proceedings of the 22nd ACM SIGPLAN-SIGACT symposium
//
// Error Handling:
// - Reference: [Composable Error Handling] - Various FP literature
//   EitherT as the standard approach to typed error composition
//
// ============================================================================
// EITHERT MONAD TRANSFORMER OVERVIEW
// ============================================================================
//
// KEY INSIGHT: EitherT<M, L, R> = M<Either<L, R>>
//
// EitherT layers error handling on top of any monad M:
// - If the inner monad produces Left(error), computation short-circuits
// - If the inner monad produces Right(value), computation continues
// - Combines typed error handling with the effects of the base monad M
//
// STRUCTURE:
// - runEitherT: Unwrap to get M<Either<L, R>>
// - lift: Lift M<A> to EitherT<M, L, A> by wrapping in Right
// - left: Construct an error (Left value)
// - right: Construct a success (Right value)
// - Functor/Applicative/Monad instances compose both layers
//
// TYPICAL USAGE:
// - EitherT<IO, Error, User>: IO operations with typed error handling
// - EitherT<State, Error, Value>: Stateful computations that can fail
// - EitherT<Reader, Error, Config>: Configuration with validation errors
//
// ============================================================================
// MONAD LAWS FOR EITHERT
// ============================================================================
//
// Standard Monad Laws (for EitherT<M, L, R> where M is a Monad):
// 1. Left Identity: return a >>= f ≡ f a
// 2. Right Identity: m >>= return ≡ m
// 3. Associativity: (m >>= f) >>= g ≡ m >>= (λx -> f x >>= g)
//
// Transformer-Specific Laws:
// 4. lift . return ≡ return
// 5. lift (m >>= f) ≡ lift m >>= (lift . f)
//
// Error Propagation:
// 6. left e >>= f ≡ left e (Left short-circuits)
// 7. right a >>= f ≡ f a (Right continues)
//
// ============================================================================

#pragma once

#include <fp20/monads/either.hpp>
#include <functional>
#include <type_traits>
#include <utility>
#include <concepts>

namespace fp20 {

    // Forward declarations
    template<typename Func, typename M, typename A>
        requires requires(Func f, M m) { fmap(f, m); }
    auto fmap(Func&& f, M&& m);

    template<typename M, typename A, typename Func>
        requires requires(M m, Func f) { bind(m, f); }
    auto bind(M&& m, Func&& f);

    // ========================================================================
    // EITHERT TYPE - Error Transformer
    // ========================================================================
    // EitherT<M, L, R> represents a computation in monad M that may fail with error L.
    // Internally: M<Either<L, R>>
    //
    // The value_ member is the inner monad containing an Either value.
    template<template<typename> class M, typename L, typename R>
    class EitherT {
    public:
        using value_type = R;
        using error_type = L;
        using inner_monad_type = M<Either<L, R>>;

    private:
        inner_monad_type value_;

    public:
        // ====================================================================
        // CONSTRUCTORS
        // ====================================================================

        // Construct EitherT from the inner monad M<Either<L, R>>
        // Time complexity: O(1) + O(M constructor)
        // Space complexity: O(size of M<Either<L, R>>)
        explicit EitherT(inner_monad_type value)
            : value_(std::move(value)) {}

        // Copy/move semantics (defaulted)
        EitherT(const EitherT&) = default;
        EitherT(EitherT&&) noexcept = default;
        EitherT& operator=(const EitherT&) = default;
        EitherT& operator=(EitherT&&) noexcept = default;

        // ====================================================================
        // UNWRAPPING
        // ====================================================================

        // runEitherT :: EitherT m l r -> m (Either l r)
        // Extract the inner monad computation.
        // Returns: M<Either<L, R>>
        //
        // Time complexity: O(copy/move M)
        // Space complexity: O(size of M)
        [[nodiscard]] inner_monad_type runEitherT() const {
            return value_;
        }

        // Non-const version for move extraction
        [[nodiscard]] inner_monad_type runEitherT() {
            return std::move(value_);
        }
    };

    // ========================================================================
    // SMART CONSTRUCTORS
    // ========================================================================

    // eitherT :: m (Either l r) -> EitherT m l r
    // Wrap an M<Either<L, R>> into EitherT.
    //
    // Time complexity: O(1)
    // Space complexity: O(1)
    template<template<typename> class M, typename L, typename R>
    EitherT<M, L, R> eitherT(M<Either<L, R>> inner) {
        return EitherT<M, L, R>(std::move(inner));
    }

    // left :: l -> EitherT m l r
    // Construct an EitherT representing an error (Left).
    //
    // Time complexity: O(pure on M)
    // Space complexity: O(M + L)
    template<template<typename> class M, typename L, typename R>
        requires requires(L l) {
            { pure<M, Either<L, R>>(Either<L, R>::left(l)) } -> std::convertible_to<M<Either<L, R>>>;
        }
    EitherT<M, L, R> left(L error) {
        return EitherT<M, L, R>(pure<M, Either<L, R>>(Either<L, R>::left(std::move(error))));
    }

    // right :: r -> EitherT m l r
    // Construct an EitherT representing success (Right).
    //
    // Time complexity: O(pure on M)
    // Space complexity: O(M + R)
    template<template<typename> class M, typename L, typename R>
        requires requires(R r) {
            { pure<M, Either<L, R>>(Either<L, R>::right(r)) } -> std::convertible_to<M<Either<L, R>>>;
        }
    EitherT<M, L, R> right(R value) {
        return EitherT<M, L, R>(pure<M, Either<L, R>>(Either<L, R>::right(std::move(value))));
    }

    // ========================================================================
    // LIFT OPERATION - The Key Transformer Function
    // ========================================================================

    // lift :: Monad m => m a -> EitherT m l a
    // Lift a computation from the base monad M into EitherT.
    // Wraps the result in Right (always succeeds).
    //
    // This is THE fundamental operation that makes EitherT a transformer.
    //
    // Time complexity: O(fmap on M)
    // Space complexity: O(size of M)
    template<template<typename> class M, typename L, typename R>
        requires requires(M<R> mr) {
            { fmap([](R r) { return Either<L, R>::right(r); }, mr) } -> std::convertible_to<M<Either<L, R>>>;
        }
    EitherT<M, L, R> lift(M<R> mr) {
        // Lift by mapping the inner value into Right
        auto wrapped = fmap([](R r) { return Either<L, R>::right(std::move(r)); }, std::move(mr));
        return EitherT<M, L, R>(std::move(wrapped));
    }

    // ========================================================================
    // FUNCTOR INSTANCE - fmap over EitherT
    // ========================================================================

    // fmap :: (a -> b) -> EitherT m l a -> EitherT m l b
    // Transform the Right value inside EitherT, preserving both layers.
    // Left values are propagated unchanged.
    //
    // Functor Laws:
    // 1. Identity: fmap(id, et) ≡ et
    // 2. Composition: fmap(g ∘ f, et) ≡ fmap(g, fmap(f, et))
    //
    // Time complexity: O(fmap on M)
    // Space complexity: O(result size)
    template<typename Func, template<typename> class M, typename L, typename R>
        requires std::invocable<Func, R>
    auto fmap(Func&& f, const EitherT<M, L, R>& et) {
        using S = std::invoke_result_t<Func, R>;

        // Map over the inner monad, transforming the Either value
        auto inner = et.runEitherT();
        auto mapped = fmap(
            [f = std::forward<Func>(f)](Either<L, R> either) -> Either<L, S> {
                if (either.is_right()) {
                    return Either<L, S>::right(f(either.right()));
                } else {
                    return Either<L, S>::left(either.left());
                }
            },
            std::move(inner)
        );

        return EitherT<M, L, S>(std::move(mapped));
    }

    // ========================================================================
    // APPLICATIVE INSTANCE
    // ========================================================================

    // pure :: a -> EitherT m l a
    // Lift a pure value into EitherT.
    // Creates M<Either<L, R>> with Right(value).
    //
    // Requires: M has a pure operation
    //
    // Time complexity: O(pure on M)
    // Space complexity: O(size of result)
    template<template<typename> class EitherT_T, template<typename> class M, typename L, typename R>
        requires std::same_as<EitherT_T<R>, EitherT<M, L, R>> &&
                 requires(R r) {
                     { pure<M, Either<L, R>>(Either<L, R>::right(r)) } -> std::convertible_to<M<Either<L, R>>>;
                 }
    EitherT<M, L, R> pure(R value) {
        // Pure value wrapped in Right, then lifted into M
        return EitherT<M, L, R>(pure<M, Either<L, R>>(Either<L, R>::right(std::move(value))));
    }

    // apply :: EitherT m l (a -> b) -> EitherT m l a -> EitherT m l b
    // Apply a function in EitherT context to a value in EitherT context.
    // Short-circuits if either is Left.
    //
    // Time complexity: O(bind on M)
    // Space complexity: O(result size)
    template<template<typename> class M, typename L, typename Func, typename R>
        requires std::invocable<Func, R>
    auto apply(const EitherT<M, L, Func>& etf, const EitherT<M, L, R>& eta) {
        using S = std::invoke_result_t<Func, R>;

        // Bind over the function monad
        auto result = bind(
            etf.runEitherT(),
            [eta_inner = eta.runEitherT()](Either<L, Func> either_f) -> M<Either<L, S>> {
                if (either_f.is_left()) {
                    // Function is Left, propagate error
                    return pure<M, Either<L, S>>(Either<L, S>::left(either_f.left()));
                }

                // Function is Right, bind over the argument monad
                return bind(
                    eta_inner,
                    [f = either_f.right()](Either<L, R> either_a) -> M<Either<L, S>> {
                        if (either_a.is_left()) {
                            // Argument is Left, propagate error
                            return pure<M, Either<L, S>>(Either<L, S>::left(either_a.left()));
                        }

                        // Both are Right, apply function
                        return pure<M, Either<L, S>>(
                            Either<L, S>::right(f(either_a.right()))
                        );
                    }
                );
            }
        );

        return EitherT<M, L, S>(std::move(result));
    }

    // ========================================================================
    // MONAD INSTANCE
    // ========================================================================

    // bind :: EitherT m l a -> (a -> EitherT m l b) -> EitherT m l b
    // Sequences EitherT computations with short-circuiting on Left.
    //
    // The key operation for EitherT:
    // 1. Run the inner M computation to get Either<L, R>
    // 2. If Left, short-circuit and propagate error
    // 3. If Right, apply continuation k to get EitherT<M, L, S>
    // 4. Run that to get M<Either<L, S>>
    //
    // Monad Laws:
    // 1. Left Identity: bind(pure(a), k) ≡ k(a)
    // 2. Right Identity: bind(m, pure) ≡ m
    // 3. Associativity: bind(bind(m, f), g) ≡ bind(m, [](auto x) { return bind(f(x), g); })
    //
    // Time complexity: O(bind on M)
    // Space complexity: O(result size)
    template<template<typename> class M, typename L, typename R, typename Func>
        requires std::invocable<Func, R> &&
                 requires(Func f, R r) {
                     { f(r) } -> std::convertible_to<EitherT<M, L, typename std::invoke_result_t<Func, R>::value_type>>;
                 }
    auto bind(const EitherT<M, L, R>& m, Func&& k) {
        using EitherTS = std::invoke_result_t<Func, R>;
        using S = typename EitherTS::value_type;

        // Bind over the inner monad
        auto result = bind(
            m.runEitherT(),
            [k = std::forward<Func>(k)](Either<L, R> either) -> M<Either<L, S>> {
                if (either.is_left()) {
                    // Short-circuit: propagate error
                    return pure<M, Either<L, S>>(Either<L, S>::left(either.left()));
                }

                // Success: apply continuation and unwrap
                EitherTS ets = k(either.right());
                return ets.runEitherT();
            }
        );

        return EitherT<M, L, S>(std::move(result));
    }

    // return_ :: a -> EitherT m l a
    // Alias for pure (monadic return).
    template<template<typename> class EitherT_T, template<typename> class M, typename L, typename R>
        requires std::same_as<EitherT_T<R>, EitherT<M, L, R>>
    EitherT<M, L, R> return_(R value) {
        return pure<EitherT_T, M, L, R>(std::move(value));
    }

    // ========================================================================
    // EITHERT-SPECIFIC OPERATIONS
    // ========================================================================

    // fromEither :: Either l r -> EitherT m l r
    // Lift a plain Either into EitherT.
    //
    // Time complexity: O(pure on M)
    // Space complexity: O(M)
    template<template<typename> class M, typename L, typename R>
        requires requires(Either<L, R> either) {
            { pure<M, Either<L, R>>(either) } -> std::convertible_to<M<Either<L, R>>>;
        }
    EitherT<M, L, R> fromEither(Either<L, R> either) {
        return EitherT<M, L, R>(pure<M, Either<L, R>>(std::move(either)));
    }

    // mapLeft :: (l -> l') -> EitherT m l r -> EitherT m l' r
    // Transform the error type (Left value).
    // Right values are propagated unchanged.
    //
    // Time complexity: O(fmap on M)
    // Space complexity: O(result size)
    template<typename Func, template<typename> class M, typename L, typename R>
        requires std::invocable<Func, L>
    auto mapLeft(Func&& f, const EitherT<M, L, R>& et) {
        using L_new = std::invoke_result_t<Func, L>;

        auto mapped = fmap(
            [f = std::forward<Func>(f)](Either<L, R> either) -> Either<L_new, R> {
                if (either.is_left()) {
                    return Either<L_new, R>::left(f(either.left()));
                } else {
                    return Either<L_new, R>::right(either.right());
                }
            },
            et.runEitherT()
        );

        return EitherT<M, L_new, R>(std::move(mapped));
    }

    // swapEitherT :: EitherT m l r -> EitherT m r l
    // Swap Left and Right.
    //
    // Time complexity: O(fmap on M)
    // Space complexity: O(result size)
    template<template<typename> class M, typename L, typename R>
    EitherT<M, R, L> swapEitherT(const EitherT<M, L, R>& et) {
        auto swapped = fmap(
            [](Either<L, R> either) -> Either<R, L> {
                if (either.is_left()) {
                    return Either<R, L>::right(either.left());
                } else {
                    return Either<R, L>::left(either.right());
                }
            },
            et.runEitherT()
        );

        return EitherT<M, R, L>(std::move(swapped));
    }

    // mapEitherT :: (m (Either l r) -> n (Either l' r')) -> EitherT m l r -> EitherT n l' r'
    // Transform the inner monad computation.
    // Allows morphing between different base monads and error types.
    //
    // Time complexity: O(f)
    // Space complexity: O(result)
    template<template<typename> class M, template<typename> class N, typename L, typename R, typename L_new, typename R_new, typename Func>
        requires std::invocable<Func, M<Either<L, R>>> &&
                 std::convertible_to<std::invoke_result_t<Func, M<Either<L, R>>>, N<Either<L_new, R_new>>>
    EitherT<N, L_new, R_new> mapEitherT(Func&& f, const EitherT<M, L, R>& et) {
        return EitherT<N, L_new, R_new>(f(et.runEitherT()));
    }

    // fromRight :: r -> EitherT m l r -> m r
    // Run EitherT and provide a default value if it's Left.
    //
    // Time complexity: O(fmap on M)
    // Space complexity: O(M)
    template<template<typename> class M, typename L, typename R>
        requires requires(M<Either<L, R>> m, R default_val) {
            { fmap([default_val](Either<L, R> either) {
                return either.is_right() ? either.right() : default_val;
            }, m) } -> std::convertible_to<M<R>>;
        }
    M<R> fromRight(R default_value, const EitherT<M, L, R>& et) {
        return fmap(
            [default_value = std::move(default_value)](Either<L, R> either) {
                return either.is_right() ? either.right() : std::move(default_value);
            },
            et.runEitherT()
        );
    }

    // ========================================================================
    // HELPER FUNCTIONS FOR ERROR HANDLING
    // ========================================================================

    // catchE :: EitherT m l a -> (l -> EitherT m l a) -> EitherT m l a
    // Catch and handle Left values (error recovery).
    //
    // Time complexity: O(bind on M)
    // Space complexity: O(result size)
    template<template<typename> class M, typename L, typename R, typename Func>
        requires std::invocable<Func, L> &&
                 std::convertible_to<std::invoke_result_t<Func, L>, EitherT<M, L, R>>
    EitherT<M, L, R> catchE(const EitherT<M, L, R>& et, Func&& handler) {
        auto result = bind(
            et.runEitherT(),
            [handler = std::forward<Func>(handler)](Either<L, R> either) -> M<Either<L, R>> {
                if (either.is_left()) {
                    // Handle the error
                    EitherT<M, L, R> recovered = handler(either.left());
                    return recovered.runEitherT();
                } else {
                    // Success, propagate
                    return pure<M, Either<L, R>>(std::move(either));
                }
            }
        );

        return EitherT<M, L, R>(std::move(result));
    }

    // ========================================================================
    // USAGE PATTERNS AND EXAMPLES
    // ========================================================================
    //
    // 1. BASIC EITHERT WITH IO:
    // ```cpp
    // auto fetchUser = [](int id) -> EitherT<IO, std::string, User> {
    //     auto io_result = effect([id]() {
    //         auto result = database_lookup(id);
    //         if (result.has_value()) {
    //             return Either<std::string, User>::right(*result);
    //         } else {
    //             return Either<std::string, User>::left("User not found");
    //         }
    //     });
    //     return EitherT<IO, std::string, User>(io_result);
    // };
    //
    // auto result = bind(fetchUser(42), [](User user) {
    //     return right<IO, std::string>(user.name);
    // });
    //
    // auto io_action = result.runEitherT();
    // Either<std::string, std::string> name = io_action.unsafeRun();
    // ```
    //
    // 2. ERROR PROPAGATION:
    // ```cpp
    // auto validate_age = [](User user) -> EitherT<IO, std::string, User> {
    //     if (user.age >= 18) {
    //         return right<IO, std::string>(user);
    //     } else {
    //         return left<IO, User>(std::string{"User under 18"});
    //     }
    // };
    //
    // auto pipeline = bind(fetchUser(42), validate_age);
    // // If user not found OR under 18, entire pipeline returns Left
    // ```
    //
    // 3. ERROR RECOVERY WITH CATCHE:
    // ```cpp
    // auto fallback = [](std::string error) -> EitherT<IO, std::string, User> {
    //     return right<IO, std::string>(User::guest());
    // };
    //
    // auto safe_fetch = catchE(fetchUser(42), fallback);
    // // Always returns a User (guest if error occurs)
    // ```
    //
    // 4. LIFTING BASE MONAD INTO EITHERT:
    // ```cpp
    // IO<int> compute = effect([]{ return 42; });
    // EitherT<IO, std::string, int> lifted = lift<IO, std::string>(std::move(compute));
    // // Wraps result in Right automatically
    // ```
    //
    // ========================================================================

} // namespace fp20
