// ============================================================================
// FP++20: Reader<E, A> Monad Implementation
// ============================================================================
// The Reader monad encapsulates computations that depend on a shared read-only
// environment. It provides elegant dependency injection through monadic composition,
// eliminating explicit threading of configuration/context through functions.
//
// ============================================================================
// ACADEMIC REFERENCES & FOUNDATIONAL LITERATURE
// ============================================================================
//
// Haskell Implementation:
// - Reference: [Control.Monad.Reader] - The mtl package
//   https://hackage.haskell.org/package/mtl/docs/Control-Monad-Reader.html
//   The canonical Reader monad showing ask, asks, local primitives
//
// - Reference: [Reader Monad Documentation]
//   https://wiki.haskell.org/All_About_Monads#The_Reader_monad
//   Comprehensive guide to Reader usage patterns
//
// Category Theory Foundation:
// - Reference: [Monads for Functional Programming] - Philip Wadler (1995)
//   Advanced Functional Programming, Springer LNCS 925
//   Section on Reader as the environment monad
//
// - Reference: [Notions of Computation and Monads] - Eugenio Moggi (1991)
//   Information and Computation 93(1)
//   Foundational paper establishing monads for computational effects
//
// Functional Programming Patterns:
// - Reference: [Dependency Injection in Functional Programming]
//   https://wiki.haskell.org/Dependency_injection
//   Reader monad as the functional solution to DI
//
// - Reference: [Functional Programming with Bananas, Lenses, Envelopes and Barbed Wire]
//   Meijer et al. (1991) - Reader as dual to Writer
//
// Category Theory Perspective:
// - Reader r a ≅ r → a (function type)
// - Functor instance: post-composition (fmap f . g = f ∘ g)
// - Monad instance: Kleisli composition with shared environment
// - Reader forms a comonad in the dual category
//
// ============================================================================
// READER MONAD OVERVIEW
// ============================================================================
//
// KEY INSIGHT: Reader<E, A> ≅ E → A (wraps a function from environment to value)
//
// The Reader monad represents computations that:
// 1. Read values from a shared environment of type E
// 2. Produce a result of type A
// 3. Cannot modify the environment (read-only)
//
// CORE OPERATIONS:
// - ask: Retrieve the entire environment
// - asks: Project a value from the environment
// - local: Execute a computation with a modified environment
//
// MONADIC STRUCTURE:
// - Functor: Transform the result while preserving environment access
// - Applicative: Share environment between function and argument
// - Monad: Thread environment through sequential computations
//
// ============================================================================
// MONAD LAWS FOR READER
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
// Reader-Specific Laws:
// 4. ask-ask: ask >>= \x -> ask >>= \y -> f x y  ≡  ask >>= \x -> f x x
//    Reading environment twice gives same value
//
// 5. local-ask: local f ask  ≡  fmap f ask
//    Modifying then reading is same as reading then transforming
//
// 6. local-local: local f (local g m)  ≡  local (g ∘ f) m
//    Nested local modifications compose
//
// 7. local-bind: local f (m >>= k)  ≡  local f m >>= (local f ∘ k)
//    local distributes over bind
//
// ============================================================================

#pragma once

#include <functional>
#include <type_traits>
#include <utility>

namespace fp20 {

    // ========================================================================
    // READER TYPE - Function from Environment to Value
    // ========================================================================
    // Reader<E, A> wraps a function E → A
    // This is the fundamental structure underlying dependency injection
    template<typename E, typename A>
    class Reader {
    public:
        using env_type = E;
        using value_type = A;

    private:
        using ReaderFunc = std::function<A(E)>;
        ReaderFunc runReaderF;

    public:
        // Construct Reader from a function E → A
        explicit Reader(ReaderFunc f) : runReaderF(std::move(f)) {}

        // Run the Reader computation with the given environment
        // This extracts the value by applying the environment
        A runReader(E env) const {
            return runReaderF(env);
        }
    };

    // ========================================================================
    // FUNCTOR INSTANCE - Post-Composition
    // ========================================================================
    // fmap :: (a → b) → Reader e a → Reader e b
    // fmap f reader = Reader (\e -> f (runReader reader e))
    //
    // Transforms the result value while preserving environment access.
    // Mathematically: fmap f = (f ∘)  (post-composition)
    //
    // Time complexity: O(1) to construct, O(f + runReader) to execute
    // Space complexity: O(1)
    template<typename Func, typename E, typename A>
    auto fmap(Func&& f, const Reader<E, A>& reader) {
        using B = std::decay_t<decltype(f(std::declval<A>()))>;

        return Reader<E, B>([reader, f = std::forward<Func>(f)](E env) -> B {
            return f(reader.runReader(env));
        });
    }

    // ========================================================================
    // APPLICATIVE INSTANCE - Shared Environment
    // ========================================================================

    // pure :: a → Reader e a
    // pure x = Reader (\_ -> x)
    //
    // Creates a Reader that ignores the environment and returns a constant value.
    // This is the minimal/trivial Reader computation.
    //
    // Time complexity: O(1)
    // Space complexity: O(1)
    template<typename E, typename A>
    Reader<E, A> pure(A value) {
        return Reader<E, A>([v = std::move(value)](E) -> A {
            return v;
        });
    }

    // Explicit two-template-parameter version for clarity
    template<typename E, typename A>
    Reader<E, A> return_(A value) {
        return pure<E, A>(std::move(value));
    }

    // apply :: Reader e (a → b) → Reader e a → Reader e b
    // apply rf rx = Reader (\e -> runReader rf e (runReader rx e))
    //
    // Applies a function in Reader context to a value in Reader context.
    // Both computations share the same environment.
    //
    // Time complexity: O(1) to construct, O(rf + rx) to execute
    // Space complexity: O(1)
    template<typename E, typename Func, typename A>
    auto apply(const Reader<E, Func>& rf, const Reader<E, A>& rx) {
        using B = std::decay_t<decltype(std::declval<Func>()(std::declval<A>()))>;

        return Reader<E, B>([rf, rx](E env) -> B {
            auto f = rf.runReader(env);
            auto x = rx.runReader(env);
            return f(x);
        });
    }

    // ========================================================================
    // MONAD INSTANCE - Environment Threading
    // ========================================================================

    // bind :: Reader e a → (a → Reader e b) → Reader e b
    // bind m k = Reader (\e -> runReader (k (runReader m e)) e)
    //
    // Sequences computations, threading the same environment through all steps.
    // The continuation k can use the value from m to determine what to compute next.
    //
    // Time complexity: O(1) to construct, O(m + k) to execute
    // Space complexity: O(1)
    template<typename E, typename A, typename Func>
    auto bind(const Reader<E, A>& m, Func&& k)
    {
        using ReaderB = std::decay_t<decltype(k(std::declval<A>()))>;
        using B = typename ReaderB::value_type;

        return Reader<E, B>([m, k = std::forward<Func>(k)](E env) mutable -> B {
            A a = m.runReader(env);
            ReaderB mb = k(a);
            return mb.runReader(env);
        });
    }

    // ========================================================================
    // READER PRIMITIVES - Core Operations
    // ========================================================================

    // ask :: Reader e e
    // ask = Reader (\e -> e)
    //
    // Retrieves the entire environment as the result value.
    // This is the fundamental primitive for accessing the environment.
    //
    // Time complexity: O(1)
    // Space complexity: O(1)
    template<typename E>
    Reader<E, E> ask() {
        return Reader<E, E>([](E env) -> E {
            return env;
        });
    }

    // asks :: (e → a) → Reader e a
    // asks f = Reader f
    //
    // Projects a value from the environment using the given function.
    // Useful for extracting specific fields from configuration records.
    //
    // Time complexity: O(1) to construct, O(f) to execute
    // Space complexity: O(1)
    template<typename E, typename Func>
    auto asks(Func&& f) {
        using A = std::decay_t<decltype(f(std::declval<E>()))>;

        return Reader<E, A>([f = std::forward<Func>(f)](E env) -> A {
            return f(env);
        });
    }

    // local :: (e → e) → Reader e a → Reader e a
    // local f m = Reader (\e -> runReader m (f e))
    //
    // Executes a Reader computation with a modified environment.
    // The modification is local to this computation; the original environment
    // is unchanged for other parts of the program.
    //
    // This enables temporary environment overrides (e.g., test configurations).
    //
    // Time complexity: O(1) to construct, O(f + m) to execute
    // Space complexity: O(1)
    template<typename E, typename Func, typename A>
    Reader<E, A> local(Func&& f, const Reader<E, A>& m) {
        return Reader<E, A>([f = std::forward<Func>(f), m](E env) -> A {
            E modified_env = f(env);
            return m.runReader(modified_env);
        });
    }

    // ========================================================================
    // USAGE PATTERNS AND EXAMPLES
    // ========================================================================
    //
    // 1. BASIC CONFIGURATION ACCESS:
    // ```cpp
    // struct Config { std::string host; int port; };
    // auto getHost = asks<Config>([](const Config& c) { return c.host; });
    // auto getPort = asks<Config>([](const Config& c) { return c.port; });
    // ```
    //
    // 2. BUILDING CONNECTION STRINGS:
    // ```cpp
    // auto connectionString = bind(
    //     asks<Config>([](auto& c) { return c.host; }),
    //     [](auto host) {
    //         return bind(
    //             asks<Config>([](auto& c) { return c.port; }),
    //             [host](auto port) {
    //                 return pure<Config>(host + ":" + std::to_string(port));
    //             }
    //         );
    //     }
    // );
    // ```
    //
    // 3. TESTING WITH MODIFIED CONFIGURATION:
    // ```cpp
    // auto production = getConnectionLimit;
    // auto testing = local(
    //     [](Config c) { c.max_connections = 5; return c; },
    //     getConnectionLimit
    // );
    // ```
    //
    // 4. DEPENDENCY INJECTION:
    // ```cpp
    // auto processWithLogging = bind(
    //     logMessage("Starting"),
    //     [](auto msg) {
    //         return bind(doWork(), [msg](auto result) {
    //             return logMessage("Done");
    //         });
    //     }
    // );
    // // Logger injected via Reader environment
    // ```
    //
    // ========================================================================

} // namespace fp20
