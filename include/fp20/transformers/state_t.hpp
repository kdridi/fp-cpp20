// ============================================================================
// FP++20: StateT<S, M, A> Monad Transformer Implementation
// ============================================================================
// StateT adds stateful computation semantics to any monad M.
// StateT s m a ≅ s → m (a, s)
//
// This allows composing stateful computations with other effects like IO, Either, etc.
//
// ============================================================================
// ACADEMIC REFERENCES & FOUNDATIONAL LITERATURE
// ============================================================================
//
// Monad Transformers:
// - Reference: [Monad Transformers Step by Step] - Martin Grabmüller (2006)
//   https://page.mi.fu-berlin.de/scravy/realworldhaskell/materialien/monad-transformers-step-by-step.pdf
//
// - Reference: [Haskell StateT] - Control.Monad.Trans.State
//   https://hackage.haskell.org/package/transformers/docs/Control-Monad-Trans-State-Strict.html
//   Standard Haskell implementation
//
// - Reference: [State Monad Transformer] - All About Monads
//   https://wiki.haskell.org/All_About_Monads#The_State_monad
//
// Stateful Computations:
// - Reference: [State Threads] - John Launchbury & Simon Peyton Jones (1994)
//   https://www.microsoft.com/en-us/research/publication/lazy-functional-state-threads/
//
// ============================================================================
// STATET MONAD TRANSFORMER OVERVIEW
// ============================================================================
//
// KEY INSIGHT: StateT<S, M, A> = S → M<(A, S)>
//
// StateT layers state threading on top of any monad M:
// - Each computation receives state and produces a value + new state in monad M
// - State flows sequentially through bind operations
// - Combines stateful computation with the effects of the base monad M
//
// STRUCTURE:
// - runStateT: Unwrap to get S → M<(A, S)>
// - lift: Lift M<A> to StateT<S, M, A> by threading state unchanged
// - get: Retrieve current state
// - put: Set new state
// - modify: Transform state
// - Functor/Applicative/Monad instances compose both layers
//
// TYPICAL USAGE:
// - StateT<Counter, IO, Result>: IO operations with mutable counter
// - StateT<AppState, Either<Error>, Value>: Stateful computations that can fail
// - StateT<GameState, Maybe, Action>: Stateful optional computations
//
// ============================================================================
// MONAD LAWS FOR STATET
// ============================================================================
//
// Standard Monad Laws (for StateT<S, M, A> where M is a Monad):
// 1. Left Identity: return a >>= f ≡ f a
// 2. Right Identity: m >>= return ≡ m
// 3. Associativity: (m >>= f) >>= g ≡ m >>= (λx -> f x >>= g)
//
// Transformer-Specific Laws:
// 4. lift . return ≡ return
// 5. lift (m >>= f) ≡ lift m >>= (lift . f)
//
// State Laws:
// 6. get >>= put ≡ return ()
// 7. put s >> get ≡ put s >> return s
// 8. put s1 >> put s2 ≡ put s2
//
// ============================================================================

#pragma once

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

    // Unit type for void-like computations
    #ifndef FP20_UNIT_DEFINED
    #define FP20_UNIT_DEFINED
    struct Unit {
        constexpr bool operator==(const Unit&) const noexcept = default;
    };
    #endif

    // ========================================================================
    // STATET TYPE - State Transformer
    // ========================================================================
    // StateT<S, M, A> represents a stateful computation in monad M.
    // Internally: S → M<(A, S)>
    //
    // The runStateF_ member is the function from state to monadic pair.
    template<typename S, template<typename> class M, typename A>
    class StateT {
    public:
        using state_type = S;
        using value_type = A;
        using result_type = M<std::pair<A, S>>;

    private:
        std::function<result_type(S)> runStateF_;

    public:
        // ====================================================================
        // CONSTRUCTORS
        // ====================================================================

        // Construct StateT from a function S → M<(A, S)>
        // Time complexity: O(1)
        // Space complexity: O(function object)
        explicit StateT(std::function<result_type(S)> f)
            : runStateF_(std::move(f)) {}

        // Construct from any callable
        template<typename F>
            requires std::invocable<F, S> &&
                     std::same_as<std::invoke_result_t<F, S>, result_type>
        explicit StateT(F&& f)
            : runStateF_(std::forward<F>(f)) {}

        // Copy/move semantics (defaulted)
        StateT(const StateT&) = default;
        StateT(StateT&&) noexcept = default;
        StateT& operator=(const StateT&) = default;
        StateT& operator=(StateT&&) noexcept = default;

        // ====================================================================
        // UNWRAPPING
        // ====================================================================

        // runStateT :: StateT s m a -> s -> m (a, s)
        // Execute the stateful computation with the given initial state.
        // Returns: M<(A, S)>
        //
        // Time complexity: O(computation)
        // Space complexity: O(M)
        [[nodiscard]] result_type runStateT(S state) const {
            return runStateF_(std::move(state));
        }
    };

    // ========================================================================
    // SMART CONSTRUCTORS
    // ========================================================================

    // stateT :: (s -> m (a, s)) -> StateT s m a
    // Wrap a function into StateT.
    //
    // Time complexity: O(1)
    // Space complexity: O(1)
    template<typename S, template<typename> class M, typename A>
    StateT<S, M, A> stateT(std::function<M<std::pair<A, S>>(S)> f) {
        return StateT<S, M, A>(std::move(f));
    }

    // ========================================================================
    // STATE PRIMITIVES
    // ========================================================================

    // get :: StateT s m s
    // Retrieve the current state as the result value.
    //
    // Time complexity: O(pure on M)
    // Space complexity: O(M)
    template<typename S, template<typename> class M>
        requires requires(S s) {
            { pure<M, std::pair<S, S>>(std::pair<S, S>(s, s)) } -> std::convertible_to<M<std::pair<S, S>>>;
        }
    StateT<S, M, S> get() {
        return StateT<S, M, S>([](S s) -> M<std::pair<S, S>> {
            return pure<M, std::pair<S, S>>(std::pair<S, S>(s, s));
        });
    }

    // put :: s -> StateT s m Unit
    // Set the state to a new value.
    //
    // Time complexity: O(pure on M)
    // Space complexity: O(M)
    template<typename S, template<typename> class M>
        requires requires(S s) {
            { pure<M, std::pair<Unit, S>>(std::pair<Unit, S>(Unit{}, s)) } -> std::convertible_to<M<std::pair<Unit, S>>>;
        }
    StateT<S, M, Unit> put(S new_state) {
        return StateT<S, M, Unit>([new_state = std::move(new_state)](S) -> M<std::pair<Unit, S>> {
            return pure<M, std::pair<Unit, S>>(std::pair<Unit, S>(Unit{}, new_state));
        });
    }

    // modify :: (s -> s) -> StateT s m Unit
    // Modify the state using a function.
    //
    // Time complexity: O(pure on M + f)
    // Space complexity: O(M)
    template<typename S, template<typename> class M, typename F>
        requires std::invocable<F, S> &&
                 std::same_as<std::invoke_result_t<F, S>, S> &&
                 requires(S s) {
                     { pure<M, std::pair<Unit, S>>(std::pair<Unit, S>(Unit{}, s)) } -> std::convertible_to<M<std::pair<Unit, S>>>;
                 }
    StateT<S, M, Unit> modify(F&& f) {
        return StateT<S, M, Unit>([f = std::forward<F>(f)](S s) -> M<std::pair<Unit, S>> {
            return pure<M, std::pair<Unit, S>>(std::pair<Unit, S>(Unit{}, f(std::move(s))));
        });
    }

    // gets :: (s -> a) -> StateT s m a
    // Project a value from the state.
    //
    // Time complexity: O(pure on M + f)
    // Space complexity: O(M)
    template<typename S, template<typename> class M, typename F>
        requires std::invocable<F, S>
    auto gets(F&& f) -> StateT<S, M, std::invoke_result_t<F, S>> {
        using A = std::invoke_result_t<F, S>;

        return StateT<S, M, A>([f = std::forward<F>(f)](S s) -> M<std::pair<A, S>> {
            return pure<M, std::pair<A, S>>(std::pair<A, S>(f(s), s));
        });
    }

    // ========================================================================
    // LIFT OPERATION - The Key Transformer Function
    // ========================================================================

    // lift :: Monad m => m a -> StateT s m a
    // Lift a computation from the base monad M into StateT.
    // The state is threaded through unchanged.
    //
    // This is THE fundamental operation that makes StateT a transformer.
    //
    // Time complexity: O(fmap on M)
    // Space complexity: O(M)
    template<typename S, template<typename> class M, typename A>
        requires requires(M<A> ma, S s) {
            { fmap([s](A a) { return std::pair<A, S>(a, s); }, ma) } -> std::convertible_to<M<std::pair<A, S>>>;
        }
    StateT<S, M, A> lift(M<A> ma) {
        return StateT<S, M, A>([ma = std::move(ma)](S s) -> M<std::pair<A, S>> {
            // Thread state through unchanged
            return fmap([s](A a) { return std::pair<A, S>(std::move(a), s); }, ma);
        });
    }

    // ========================================================================
    // FUNCTOR INSTANCE - fmap over StateT
    // ========================================================================

    // fmap :: (a -> b) -> StateT s m a -> StateT s m b
    // Transform the result value, preserving state threading.
    //
    // Functor Laws:
    // 1. Identity: fmap(id, st) ≡ st
    // 2. Composition: fmap(g ∘ f, st) ≡ fmap(g, fmap(f, st))
    //
    // Time complexity: O(fmap on M)
    // Space complexity: O(result)
    template<typename Func, typename S, template<typename> class M, typename A>
        requires std::invocable<Func, A>
    auto fmap(Func&& f, const StateT<S, M, A>& st) {
        using B = std::invoke_result_t<Func, A>;

        return StateT<S, M, B>([st, f = std::forward<Func>(f)](S s) -> M<std::pair<B, S>> {
            // Run the inner StateT and map over the result
            auto m_pair = st.runStateT(std::move(s));
            return fmap(
                [f](std::pair<A, S> p) -> std::pair<B, S> {
                    return std::pair<B, S>(f(std::move(p.first)), std::move(p.second));
                },
                std::move(m_pair)
            );
        });
    }

    // ========================================================================
    // APPLICATIVE INSTANCE
    // ========================================================================

    // pure :: a -> StateT s m a
    // Lift a pure value into StateT.
    // State is threaded through unchanged.
    //
    // Time complexity: O(pure on M)
    // Space complexity: O(M)
    template<template<typename> class StateT_T, typename S, template<typename> class M, typename A>
        requires std::same_as<StateT_T<A>, StateT<S, M, A>> &&
                 requires(A a, S s) {
                     { pure<M, std::pair<A, S>>(std::pair<A, S>(a, s)) } -> std::convertible_to<M<std::pair<A, S>>>;
                 }
    StateT<S, M, A> pure(A value) {
        return StateT<S, M, A>([value = std::move(value)](S s) -> M<std::pair<A, S>> {
            return pure<M, std::pair<A, S>>(std::pair<A, S>(value, std::move(s)));
        });
    }

    // apply :: StateT s m (a -> b) -> StateT s m a -> StateT s m b
    // Apply a function in StateT context to a value in StateT context.
    // State is threaded from stf to sta.
    //
    // Time complexity: O(bind on M)
    // Space complexity: O(result)
    template<typename S, template<typename> class M, typename Func, typename A>
        requires std::invocable<Func, A>
    auto apply(const StateT<S, M, Func>& stf, const StateT<S, M, A>& sta) {
        using B = std::invoke_result_t<Func, A>;

        return StateT<S, M, B>([stf, sta](S s) -> M<std::pair<B, S>> {
            // Run stf to get function and new state
            auto m_pair_f = stf.runStateT(std::move(s));

            // Bind to sequence the effects
            return bind(
                std::move(m_pair_f),
                [sta](std::pair<Func, S> p_f) -> M<std::pair<B, S>> {
                    auto [f, s1] = std::move(p_f);

                    // Run sta with the new state
                    auto m_pair_a = sta.runStateT(std::move(s1));

                    // Map to apply the function
                    return fmap(
                        [f = std::move(f)](std::pair<A, S> p_a) -> std::pair<B, S> {
                            return std::pair<B, S>(f(std::move(p_a.first)), std::move(p_a.second));
                        },
                        std::move(m_pair_a)
                    );
                }
            );
        });
    }

    // ========================================================================
    // MONAD INSTANCE
    // ========================================================================

    // bind :: StateT s m a -> (a -> StateT s m b) -> StateT s m b
    // Sequences StateT computations, threading state through both.
    //
    // The key operation for StateT:
    // 1. Run m with state s to get M<(a, s1)>
    // 2. In the monad M, extract (a, s1)
    // 3. Apply k to a to get StateT<S, M, B>
    // 4. Run that with state s1 to get M<(b, s2)>
    //
    // Monad Laws:
    // 1. Left Identity: bind(pure(a), k) ≡ k(a)
    // 2. Right Identity: bind(m, pure) ≡ m
    // 3. Associativity: bind(bind(m, f), g) ≡ bind(m, [](auto x) { return bind(f(x), g); })
    //
    // Time complexity: O(bind on M)
    // Space complexity: O(result)
    template<typename S, template<typename> class M, typename A, typename Func>
        requires std::invocable<Func, A> &&
                 requires(Func f, A a) {
                     { f(a) } -> std::convertible_to<StateT<S, M, typename std::invoke_result_t<Func, A>::value_type>>;
                 }
    auto bind(const StateT<S, M, A>& m, Func&& k) {
        using StateTB = std::invoke_result_t<Func, A>;
        using B = typename StateTB::value_type;

        return StateT<S, M, B>([m, k = std::forward<Func>(k)](S s) -> M<std::pair<B, S>> {
            // Run the first computation
            auto m_pair_a = m.runStateT(std::move(s));

            // Bind in the inner monad
            return bind(
                std::move(m_pair_a),
                [k](std::pair<A, S> p) -> M<std::pair<B, S>> {
                    auto [a, s_new] = std::move(p);

                    // Apply continuation to get new StateT
                    StateTB stb = k(std::move(a));

                    // Run it with the new state
                    return stb.runStateT(std::move(s_new));
                }
            );
        });
    }

    // return_ :: a -> StateT s m a
    // Alias for pure (monadic return).
    template<template<typename> class StateT_T, typename S, template<typename> class M, typename A>
        requires std::same_as<StateT_T<A>, StateT<S, M, A>>
    StateT<S, M, A> return_(A value) {
        return pure<StateT_T, S, M, A>(std::move(value));
    }

    // ========================================================================
    // HELPER FUNCTIONS
    // ========================================================================

    // evalStateT :: StateT s m a -> s -> m a
    // Run StateT and return only the result value (discard final state).
    //
    // Time complexity: O(fmap on M)
    // Space complexity: O(M)
    template<typename S, template<typename> class M, typename A>
    M<A> evalStateT(const StateT<S, M, A>& st, S initial_state) {
        auto m_pair = st.runStateT(std::move(initial_state));
        return fmap(
            [](std::pair<A, S> p) -> A {
                return std::move(p.first);
            },
            std::move(m_pair)
        );
    }

    // execStateT :: StateT s m a -> s -> m s
    // Run StateT and return only the final state (discard result value).
    //
    // Time complexity: O(fmap on M)
    // Space complexity: O(M)
    template<typename S, template<typename> class M, typename A>
    M<S> execStateT(const StateT<S, M, A>& st, S initial_state) {
        auto m_pair = st.runStateT(std::move(initial_state));
        return fmap(
            [](std::pair<A, S> p) -> S {
                return std::move(p.second);
            },
            std::move(m_pair)
        );
    }

    // mapStateT :: (m (a, s) -> n (b, s)) -> StateT s m a -> StateT s n b
    // Transform the inner monad computation.
    //
    // Time complexity: O(f)
    // Space complexity: O(result)
    template<typename S, template<typename> class M, template<typename> class N, typename A, typename B, typename Func>
        requires std::invocable<Func, M<std::pair<A, S>>> &&
                 std::convertible_to<std::invoke_result_t<Func, M<std::pair<A, S>>>, N<std::pair<B, S>>>
    StateT<S, N, B> mapStateT(Func&& f, const StateT<S, M, A>& st) {
        return StateT<S, N, B>([st, f = std::forward<Func>(f)](S s) -> N<std::pair<B, S>> {
            return f(st.runStateT(std::move(s)));
        });
    }

    // ========================================================================
    // USAGE PATTERNS AND EXAMPLES
    // ========================================================================
    //
    // 1. BASIC STATET WITH IO:
    // ```cpp
    // using CounterIO = StateT<int, IO, std::string>;
    //
    // auto increment_and_log = bind(
    //     get<int, IO>(),
    //     [](int count) {
    //         return bind(
    //             put<int, IO>(count + 1),
    //             [count](Unit) {
    //                 return lift<int, IO>(putStrLn("Count: " + std::to_string(count)));
    //             }
    //         );
    //     }
    // );
    //
    // auto io_action = increment_and_log.runStateT(0);
    // auto [result, final_state] = io_action.unsafeRun();
    // ```
    //
    // 2. STATEFUL COMPUTATION WITH ERROR HANDLING:
    // ```cpp
    // using StatefulEither = StateT<AppState, EitherT<IO, Error>, User>;
    //
    // auto process = bind(
    //     get<AppState, EitherT<IO, Error>>(),
    //     [](AppState state) {
    //         return lift<AppState, EitherT<IO, Error>>(
    //             validate_user(state.current_user)
    //         );
    //     }
    // );
    // ```
    //
    // 3. MULTIPLE STATE OPERATIONS:
    // ```cpp
    // auto complex = bind(
    //     modify<GameState, IO>([](GameState gs) {
    //         gs.score += 10;
    //         return gs;
    //     }),
    //     [](Unit) {
    //         return gets<GameState, IO>([](const GameState& gs) {
    //             return gs.score;
    //         });
    //     }
    // );
    // ```
    //
    // ========================================================================

} // namespace fp20
