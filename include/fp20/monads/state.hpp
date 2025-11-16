// ============================================
// FP++20: State<S,A> Monad Implementation
// ============================================
// A stateful computation monad implementing S -> (A, S) semantics.
// State<S,A> encapsulates computations that thread state sequentially
// through a series of operations, enabling clean functional state management.
//
// ============================================
// ACADEMIC REFERENCES & FOUNDATIONAL LITERATURE
// ============================================
// Category Theory:
// - Reference: [State Monad] - https://ncatlab.org/nlab/show/state+monad
// - Reference: [Monad (in computer science)] - https://en.wikipedia.org/wiki/Monad_(functional_programming)
// - Reference: [Category Theory for Programmers] - Bartosz Milewski
//   https://bartoszmilewski.com/2016/11/30/lawvere-theories/
//
// Functional Programming:
// - Reference: [Haskell State Monad] - https://hackage.haskell.org/package/mtl/docs/Control-Monad-State.html
// - Reference: [State Monad Tutorial] - https://wiki.haskell.org/State_Monad
// - Reference: [Monads for Functional Programming] - Philip Wadler (1995)
//   https://homepages.inf.ed.ac.uk/wadler/papers/marktoberdorf/baastad.pdf
//   Section 2.9: State Monads
// - Reference: [Tackling the Awkward Squad] - Simon Peyton Jones (2001)
//   https://www.microsoft.com/en-us/research/publication/tackling-awkward-squad-monadic-inputoutput-concurrency-exceptions-foreign-language-calls-haskell/
//
// State Threading Semantics:
// - Reference: [State Threads] - John Launchbury & Simon Peyton Jones (1994)
//   https://www.microsoft.com/en-us/research/publication/lazy-functional-state-threads/
//
// C++ Design:
// - Reference: [C++20 Concepts] - https://en.cppreference.com/w/cpp/language/constraints
// - Reference: [Perfect Forwarding] - https://en.cppreference.com/w/cpp/utility/forward
// - Reference: [std::invoke] - https://en.cppreference.com/w/cpp/utility/functional/invoke
//
// ============================================
// STATE MONAD SEMANTICS
// ============================================
// State<S,A> represents a stateful computation with signature: S -> (A, S)
// - S: State type (threaded through computations)
// - A: Result value type (computed from state)
// - runState: Execute computation with initial state
//
// Key Operations:
// - get: Retrieves current state as value
// - put: Sets new state
// - modify: Transforms state via function
// - gets: Projects value from state
//
// Threading Behavior:
// - Each computation receives state from previous computation
// - State flows sequentially through bind chain
// - No implicit state sharing or backtracking
//
// ============================================
// MONAD LAWS FOR STATE
// ============================================
// 1. Left Identity: return a >>= f ≡ f a
//    - runState(bind(return_(a), f), s) == runState(f(a), s)
//
// 2. Right Identity: m >>= return ≡ m
//    - runState(bind(m, return_), s) == runState(m, s)
//
// 3. Associativity: (m >>= f) >>= g ≡ m >>= (λx -> f x >>= g)
//    - runState(bind(bind(m, f), g), s) == runState(bind(m, [](auto x){ return bind(f(x), g); }), s)
//
// ============================================
// STATE-SPECIFIC LAWS
// ============================================
// 1. get-get: get >>= (λs1. get >>= (λs2. k s1 s2)) ≡ get >>= (λs. k s s)
//    - Reading state twice returns same value
//
// 2. put-get: put s >>= (λ_. get) ≡ put s >>= (λ_. return s)
//    - After putting state, get returns that value
//
// 3. put-put: put s1 >>= (λ_. put s2) ≡ put s2
//    - Sequential puts: only last one matters
//
// 4. get-put: get >>= put ≡ return ()
//    - Getting and putting same state is identity

#pragma once

#include <functional>
#include <utility>
#include <concepts>
#include <type_traits>

namespace fp20 {
    // ============================================
    // UNIT TYPE FOR VOID-LIKE COMPUTATIONS
    // ============================================
    // State operations like put and modify don't return meaningful values.
    // We use Unit as a monoidal unit type (like () in Haskell).
    struct Unit {
        constexpr bool operator==(const Unit&) const noexcept = default;
    };

    // ============================================
    // STATE<S,A> CLASS - Stateful Computation
    // ============================================
    // Encapsulates a computation: S -> (A, S)
    // S: State type (input and output)
    // A: Result value type
    template<typename S, typename A>
    class State {
    public:
        // Type aliases for concept compatibility
        using state_type = S;
        using value_type = A;

    private:
        // The underlying stateful computation
        // Signature: S -> (A, S)
        std::function<std::pair<A, S>(S)> runStateF;

    public:
        // ============================================
        // CONSTRUCTORS
        // ============================================

        // Construct from std::function
        // Most general form: accepts pre-wrapped function
        explicit State(std::function<std::pair<A, S>(S)> f)
            : runStateF(std::move(f)) {}

        // Construct from any callable (lambda, function object)
        // Uses C++20 concepts for type-safe perfect forwarding
        template<typename F>
            requires std::invocable<F, S> &&
                     std::same_as<std::invoke_result_t<F, S>, std::pair<A, S>>
        explicit State(F&& f)
            : runStateF(std::forward<F>(f)) {}

        // Copy/move semantics (defaulted)
        // std::function is copyable and movable
        State(const State&) = default;
        State(State&&) noexcept = default;
        State& operator=(const State&) = default;
        State& operator=(State&&) noexcept = default;

        // ============================================
        // COMPUTATION EXECUTION
        // ============================================

        // runState :: State S A -> S -> (A, S)
        // Execute the stateful computation with given initial state.
        // Returns: pair of (result_value, final_state)
        [[nodiscard]] std::pair<A, S> runState(S s) const {
            return runStateF(s);
        }
    };

    // ============================================
    // STATE PRIMITIVES
    // ============================================
    // Fundamental operations for state manipulation.
    // These form the basis for all stateful computations.

    // get :: State S S
    // Returns current state as the result value, without modifying state.
    // Semantics: s -> (s, s)
    template<typename S>
    inline State<S, S> get() {
        return State<S, S>([](S s) -> std::pair<S, S> {
            return {s, s};
        });
    }

    // put :: S -> State S Unit
    // Sets new state, discarding old state.
    // Returns Unit as the result (no meaningful value).
    // Semantics: new_s -> (s -> ((), new_s))
    template<typename S>
    inline State<S, Unit> put(S new_state) {
        return State<S, Unit>([new_state = std::move(new_state)](S) -> std::pair<Unit, S> {
            return {Unit{}, new_state};
        });
    }

    // modify :: (S -> S) -> State S Unit
    // Applies transformation function to current state.
    // Returns Unit as the result.
    // Semantics: f -> (s -> ((), f(s)))
    template<typename S, typename F>
        requires std::invocable<F, S> && std::same_as<std::invoke_result_t<F, S>, S>
    inline State<S, Unit> modify(F&& f) {
        return State<S, Unit>([f = std::forward<F>(f)](S s) -> std::pair<Unit, S> {
            return {Unit{}, f(std::move(s))};
        });
    }

    // gets :: (S -> A) -> State S A
    // Projects a value from current state without modifying it.
    // Like get, but applies a selector function.
    // Semantics: f -> (s -> (f(s), s))
    template<typename S, typename F>
        requires std::invocable<F, S>
    inline auto gets(F&& f) -> State<S, std::invoke_result_t<F, S>> {
        using A = std::invoke_result_t<F, S>;
        return State<S, A>([f = std::forward<F>(f)](S s) -> std::pair<A, S> {
            return {f(s), s};
        });
    }

    // ============================================
    // FUNCTOR INSTANCE FOR STATE
    // ============================================
    // fmap :: (A -> B) -> State S A -> State S B
    // Transforms the result value of a stateful computation.
    // State threading is preserved; only the value is transformed.
    //
    // Functor Laws:
    // 1. Identity: fmap(id, m) ≡ m
    // 2. Composition: fmap(g ∘ f, m) ≡ fmap(g, fmap(f, m))
    template<typename F, typename S, typename A>
        requires std::invocable<F, A>
    inline auto fmap(F&& f, const State<S, A>& state) {
        using B = std::invoke_result_t<F, A>;

        return State<S, B>([state, f = std::forward<F>(f)](S s) -> std::pair<B, S> {
            auto [a, s_new] = state.runState(std::move(s));
            return {f(std::move(a)), std::move(s_new)};
        });
    }

    // ============================================
    // APPLICATIVE INSTANCE FOR STATE
    // ============================================

    // pure :: A -> State S A
    // Creates a stateless computation that returns given value.
    // State is threaded through unchanged.
    // Semantics: a -> (s -> (a, s))
    template<template<typename...> typename M, typename S, typename A>
        requires std::same_as<M<S, A>, State<S, A>>
    inline State<S, A> pure(A a) {
        return State<S, A>([a = std::move(a)](S s) -> std::pair<A, S> {
            return {a, std::move(s)};
        });
    }

    // apply :: State S (A -> B) -> State S A -> State S B
    // Applies a stateful function to a stateful value.
    // State is threaded from sf to sa.
    // Semantics: sf -> sa -> (s -> let (f, s1) = runState sf s
    //                                  (a, s2) = runState sa s1
    //                              in (f a, s2))
    template<typename S, typename F, typename A>
        requires std::invocable<F, A>
    inline auto apply(const State<S, F>& sf, const State<S, A>& sa) {
        using B = std::invoke_result_t<F, A>;

        return State<S, B>([sf, sa](S s) -> std::pair<B, S> {
            auto [f, s1] = sf.runState(std::move(s));
            auto [a, s2] = sa.runState(std::move(s1));
            return {f(std::move(a)), std::move(s2)};
        });
    }

    // ============================================
    // MONAD INSTANCE FOR STATE
    // ============================================

    // return_ :: A -> State S A
    // Alias for pure (monadic return).
    // Named return_ to avoid C++ keyword conflict.
    template<template<typename...> typename M, typename S, typename A>
        requires std::same_as<M<S, A>, State<S, A>>
    inline State<S, A> return_(A a) {
        return pure<M, S, A>(std::move(a));
    }

    // bind :: State S A -> (A -> State S B) -> State S B
    // Sequences stateful computations with data-dependent effects.
    // The key operation: threads state from m through k.
    //
    // Semantics: m -> k -> (s -> let (a, s1) = runState m s
    //                                (b, s2) = runState (k a) s1
    //                            in (b, s2))
    //
    // State Threading:
    // 1. Execute m with initial state s, get (a, s1)
    // 2. Apply k to value a, get new State computation
    // 3. Execute k(a) with updated state s1, get (b, s2)
    // 4. Return (b, s2)
    template<typename S, typename A, typename F>
        requires std::invocable<F, A>
    inline auto bind(const State<S, A>& m, F&& k) {
        using StateB = std::invoke_result_t<F, A>;
        using B = typename StateB::value_type;

        return State<S, B>([m, k = std::forward<F>(k)](S s) -> std::pair<B, S> {
            // Execute first computation with initial state
            auto [a, s_new] = m.runState(std::move(s));

            // Apply continuation to result, execute with updated state
            return k(std::move(a)).runState(std::move(s_new));
        });
    }

    // ============================================
    // HELPER FUNCTIONS
    // ============================================

    // evalState :: State S A -> S -> A
    // Runs computation and returns ONLY the result value.
    // Discards final state (useful when state is just a means to an end).
    template<typename S, typename A>
    inline A evalState(const State<S, A>& state, S initial_state) {
        auto [value, _] = state.runState(std::move(initial_state));
        return value;
    }

    // execState :: State S A -> S -> S
    // Runs computation and returns ONLY the final state.
    // Discards result value (useful for state-only computations).
    template<typename S, typename A>
    inline S execState(const State<S, A>& state, S initial_state) {
        auto [_, final_state] = state.runState(std::move(initial_state));
        return final_state;
    }

    // ============================================
    // USAGE EXAMPLES
    // ============================================
    // Common patterns for State monad:
    //
    // 1. COUNTER INCREMENT:
    // ```cpp
    // auto increment = []() {
    //     return bind(get<int>(), [](int s) {
    //         return bind(put<int>(s + 1), [s](auto) {
    //             return return_<State, int>(s);
    //         });
    //     });
    // };
    // auto [old_value, new_state] = increment().runState(0);
    // // old_value == 0, new_state == 1
    // ```
    //
    // 2. ACCUMULATOR:
    // ```cpp
    // auto addToState = [](int x) {
    //     return modify<int>([x](int s) { return s + x; });
    // };
    // auto computation = bind(addToState(10), [](auto) {
    //     return bind(addToState(20), [](auto) {
    //         return get<int>();
    //     });
    // });
    // auto result = evalState(computation, 0);
    // // result == 30
    // ```
    //
    // 3. STATE PROJECTION:
    // ```cpp
    // struct GameState { int score; int level; };
    // auto getScore = gets<GameState>([](const GameState& gs) {
    //     return gs.score;
    // });
    // auto [score, state] = getScore.runState(GameState{42, 5});
    // // score == 42, state unchanged
    // ```
}
