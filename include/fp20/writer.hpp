// ============================================================================
// FP++20: Writer<W, A> Monad Implementation
// ============================================================================
// The Writer monad encapsulates computations that produce values along with
// accumulated output (log). It provides elegant compositional logging through
// monadic operations, eliminating explicit log threading through functions.
//
// ============================================================================
// ACADEMIC REFERENCES & FOUNDATIONAL LITERATURE
// ============================================================================
//
// Haskell Implementation:
// - Reference: [Control.Monad.Writer] - The mtl package
//   https://hackage.haskell.org/package/mtl/docs/Control-Monad-Writer.html
//   The canonical Writer monad showing tell, listen, pass primitives
//
// - Reference: [Writer Monad Documentation]
//   https://wiki.haskell.org/All_About_Monads#The_Writer_monad
//   Comprehensive guide to Writer usage patterns
//
// Category Theory Foundation:
// - Reference: [Monads for Functional Programming] - Philip Wadler (1995)
//   Advanced Functional Programming, Springer LNCS 925
//   Section on Writer as the output/logging monad
//
// - Reference: [Notions of Computation and Monads] - Eugenio Moggi (1991)
//   Information and Computation 93(1)
//   Foundational paper establishing monads for computational effects
//
// Functional Programming Patterns:
// - Reference: [Functional Programming with Bananas, Lenses, Envelopes and Barbed Wire]
//   Meijer et al. (1991) - Writer as dual to Reader
//
// - Reference: [The Essence of Functional Programming] - Philip Wadler (1992)
//   https://dl.acm.org/doi/10.1145/143165.143169
//   Shows Writer monad for accumulating output
//
// Monoid Theory:
// - Reference: [Abstract Algebra] - Dummit & Foote
//   Monoid structure required for log accumulation
//
// - Reference: [Categories for the Working Mathematician] - Saunders Mac Lane
//   DOI: 10.1007/978-1-4757-4721-8
//   Monoidal categories underlying Writer
//
// Category Theory Perspective:
// - Writer w a ≅ (a, w) with monoidal accumulation
// - Functor instance: transform value, preserve log
// - Monad instance: Kleisli composition with log concatenation
// - Writer forms a monad in the category of monoids
//
// ============================================================================
// WRITER MONAD OVERVIEW
// ============================================================================
//
// KEY INSIGHT: Writer<W, A> ≅ (A, W) where W is a Monoid
//
// The Writer monad represents computations that:
// 1. Produce a value of type A
// 2. Accumulate output of type W (must be a Monoid)
// 3. Automatically concatenate logs when composing computations
//
// CORE OPERATIONS:
// - writer: Construct Writer with value and log
// - tell: Append output to the log
// - listen: Expose the accumulated log as part of the value
// - pass: Apply a function to transform the log
// - censor: Filter or modify the log output
//
// MONADIC STRUCTURE:
// - Functor: Transform the value while preserving the log
// - Applicative: Combine values and concatenate logs
// - Monad: Thread logs through sequential computations
//
// ============================================================================
// MONAD LAWS FOR WRITER
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
// Writer-Specific Laws:
// 4. tell-tell: tell w1 >> tell w2  ≡  tell (w1 <> w2)
//    Consecutive tells concatenate logs
//
// 5. listen-tell: listen (tell w)  ≡  tell w >> return ((), w)
//    Listening to tell exposes the log
//
// 6. censor-tell: censor f (tell w)  ≡  tell (f w)
//    Censoring tell applies transformation
//
// 7. listen-pure: listen (return a)  ≡  return (a, mempty)
//    Listening to pure gives empty log
//
// Monoid Laws (for W):
// 8. Left Identity: mempty <> w  ≡  w
// 9. Right Identity: w <> mempty  ≡  w
// 10. Associativity: (w1 <> w2) <> w3  ≡  w1 <> (w2 <> w3)
//
// ============================================================================

#pragma once

#include <functional>
#include <type_traits>
#include <utility>
#include <concepts>
#include <fp20/concepts/monoid.hpp>

namespace fp20 {

    // ========================================================================
    // UNIT TYPE FOR VOID-LIKE COMPUTATIONS
    // ========================================================================
    // Writer operations like tell don't return meaningful values.
    // We use Unit as a monoidal unit type (like () in Haskell).
    // This is defined in state.hpp, but we redefine it here for Writer's use.
    // If both are included, the definition will be the same.

    #ifndef FP20_UNIT_DEFINED
    #define FP20_UNIT_DEFINED
    struct Unit {
        constexpr bool operator==(const Unit&) const noexcept = default;
    };
    #endif

    // ========================================================================
    // WRITER TYPE - Value with Accumulated Log
    // ========================================================================
    // Writer<W, A> represents a computation that produces:
    // - A value of type A
    // - An accumulated log of type W (which must be a Monoid)
    //
    // Internally: Writer<W, A> ≅ (A, W)
    template<Monoid W, typename A>
    class Writer {
    public:
        using log_type = W;
        using value_type = A;

    private:
        A value_;
        W log_;

    public:
        // ====================================================================
        // CONSTRUCTORS
        // ====================================================================

        // Construct Writer with value and log
        // Time complexity: O(1) + O(copy/move A and W)
        // Space complexity: O(size of A + size of W)
        constexpr Writer(A value, W log)
            : value_(std::move(value)), log_(std::move(log)) {}

        // Copy/move semantics (defaulted)
        Writer(const Writer&) = default;
        Writer(Writer&&) noexcept = default;
        Writer& operator=(const Writer&) = default;
        Writer& operator=(Writer&&) noexcept = default;

        // ====================================================================
        // COMPUTATION EXTRACTION
        // ====================================================================

        // runWriter :: Writer w a -> (a, w)
        // Extract both the value and the accumulated log.
        // Returns: pair of (value, log)
        //
        // Time complexity: O(copy A + copy W)
        // Space complexity: O(size of A + size of W)
        [[nodiscard]] constexpr std::pair<A, W> runWriter() const {
            return {value_, log_};
        }
    };

    // ========================================================================
    // WRITER<W, void> SPECIALIZATION
    // ========================================================================
    // Special case for Writer<W, void> where there's no meaningful value.
    // This is used by operations like tell that only append to the log.
    //
    // We use Unit internally since void cannot be a member variable type.
    template<Monoid W>
    class Writer<W, void> {
    public:
        using log_type = W;
        using value_type = void;

    private:
        Unit unit_;  // Placeholder for void
        W log_;

    public:
        // Construct from Unit and log
        constexpr Writer(Unit, W log)
            : unit_(Unit{}), log_(std::move(log)) {}

        // Copy/move semantics
        Writer(const Writer&) = default;
        Writer(Writer&&) noexcept = default;
        Writer& operator=(const Writer&) = default;
        Writer& operator=(Writer&&) noexcept = default;

        // runWriter returns pair<int, W> where int represents void
        // (0 is the canonical "void value")
        [[nodiscard]] constexpr std::pair<int, W> runWriter() const {
            return {0, log_};
        }
    };

    // ========================================================================
    // WRITER CONSTRUCTION HELPER
    // ========================================================================

    // writer :: a -> w -> Writer w a
    // Construct a Writer with the given value and log.
    // This is the fundamental constructor for Writer computations.
    //
    // Time complexity: O(1)
    // Space complexity: O(1)
    template<Monoid W, typename A>
    constexpr Writer<W, A> writer(A value, W log) {
        return Writer<W, A>(std::move(value), std::move(log));
    }

    // ========================================================================
    // FUNCTOR INSTANCE - Transform Value
    // ========================================================================

    // fmap :: (a → b) → Writer w a → Writer w b
    // fmap f (Writer a w) = Writer (f a) w
    //
    // Transforms the value while preserving the log unchanged.
    // This is the functor mapping operation.
    //
    // Functor Laws:
    // 1. Identity: fmap(id, w) ≡ w
    // 2. Composition: fmap(g ∘ f, w) ≡ fmap(g, fmap(f, w))
    //
    // Time complexity: O(f + copy/move W)
    // Space complexity: O(size of B + size of W)
    template<typename Func, Monoid W, typename A>
        requires std::invocable<Func, A>
    constexpr auto fmap(Func&& f, const Writer<W, A>& w) {
        using B = std::invoke_result_t<Func, A>;

        auto [value, log] = w.runWriter();
        return Writer<W, B>(f(std::move(value)), std::move(log));
    }

    // ========================================================================
    // APPLICATIVE INSTANCE - Pure Values and Application
    // ========================================================================

    // pure :: a → Writer w a
    // pure a = Writer a mempty
    //
    // Creates a Writer with the given value and an empty log (mempty).
    // This is the minimal/trivial Writer computation.
    //
    // Time complexity: O(mempty)
    // Space complexity: O(size of A + size of mempty W)
    template<template<typename...> typename M, Monoid W, typename A>
        requires std::same_as<M<W, A>, Writer<W, A>>
    constexpr Writer<W, A> pure(A value) {
        return Writer<W, A>(std::move(value), mempty<W>());
    }

    // Convenient alias for pure (without template template parameter)
    // Usage: pure_writer<std::string>(42)
    template<Monoid W, typename A>
    constexpr Writer<W, A> pure_writer(A value) {
        return Writer<W, A>(std::move(value), mempty<W>());
    }

    // apply :: Writer w (a → b) → Writer w a → Writer w b
    // apply (Writer f w1) (Writer a w2) = Writer (f a) (w1 <> w2)
    //
    // Applies a function in Writer context to a value in Writer context.
    // Logs are concatenated using mappend.
    //
    // Applicative Laws:
    // 1. Identity: apply(pure(id), v) ≡ v
    // 2. Composition: apply(apply(apply(pure(∘), u), v), w) ≡ apply(u, apply(v, w))
    // 3. Homomorphism: apply(pure(f), pure(x)) ≡ pure(f(x))
    // 4. Interchange: apply(u, pure(y)) ≡ apply(pure(λf.f(y)), u)
    //
    // Time complexity: O(f + mappend)
    // Space complexity: O(size of B + size of combined W)
    template<Monoid W, typename Func, typename A>
        requires std::invocable<Func, A>
    constexpr auto apply(const Writer<W, Func>& wf, const Writer<W, A>& wa) {
        using B = std::invoke_result_t<Func, A>;

        auto [f, log1] = wf.runWriter();
        auto [a, log2] = wa.runWriter();

        return Writer<W, B>(f(std::move(a)), mappend(std::move(log1), std::move(log2)));
    }

    // ========================================================================
    // MONAD INSTANCE - Sequential Composition with Log Accumulation
    // ========================================================================

    // bind :: Writer w a → (a → Writer w b) → Writer w b
    // bind (Writer a w1) k = let Writer b w2 = k a in Writer b (w1 <> w2)
    //
    // Sequences computations, threading the value and concatenating logs.
    // The continuation k can use the value from m to determine what to compute next.
    //
    // Monad Laws:
    // 1. Left Identity: bind(pure(a), k) ≡ k(a)
    // 2. Right Identity: bind(m, pure) ≡ m
    // 3. Associativity: bind(bind(m, f), g) ≡ bind(m, [](auto x) { return bind(f(x), g); })
    //
    // Time complexity: O(m + k + mappend)
    // Space complexity: O(size of B + size of combined W)
    template<Monoid W, typename A, typename Func>
        requires std::invocable<Func, A>
    constexpr auto bind(const Writer<W, A>& m, Func&& k) {
        using WriterB = std::invoke_result_t<Func, A>;
        using B = typename WriterB::value_type;

        auto [a, w1] = m.runWriter();
        auto mb = k(std::move(a));
        auto [b, w2] = mb.runWriter();

        return Writer<W, B>(std::move(b), mappend(std::move(w1), std::move(w2)));
    }

    // bind :: Writer w void → (int → Writer w b) → Writer w b
    // Specialization for Writer<W, void> where the value is represented as int
    // The continuation receives the placeholder int value (always 0)
    template<Monoid W, typename Func>
        requires std::invocable<Func, int>
    constexpr auto bind(const Writer<W, void>& m, Func&& k) {
        using WriterB = std::invoke_result_t<Func, int>;
        using B = typename WriterB::value_type;

        auto [placeholder, w1] = m.runWriter();  // placeholder is int (0)
        auto mb = k(placeholder);
        auto [b, w2] = mb.runWriter();

        return Writer<W, B>(std::move(b), mappend(std::move(w1), std::move(w2)));
    }

    // return_ :: a → Writer w a
    // Alias for pure (monadic return).
    // Named return_ to avoid C++ keyword conflict.
    template<template<typename...> typename M, Monoid W, typename A>
        requires std::same_as<M<W, A>, Writer<W, A>>
    constexpr Writer<W, A> return_(A value) {
        return pure<M, W, A>(std::move(value));
    }

    // ========================================================================
    // WRITER PRIMITIVES - Core Operations
    // ========================================================================

    // tell :: w → Writer w void
    // tell w = Writer () w
    //
    // Creates a Writer that produces no meaningful value (void) but appends
    // w to the log. This is the fundamental primitive for logging.
    //
    // Implementation: Returns Writer<W, void> which uses Unit internally.
    //
    // Time complexity: O(copy/move W)
    // Space complexity: O(size of W)
    template<Monoid W>
    constexpr Writer<W, void> tell(W log) {
        return Writer<W, void>(Unit{}, std::move(log));
    }

    // listen :: Writer w a → Writer w (a, w)
    // listen (Writer a w) = Writer (a, w) w
    //
    // Exposes the accumulated log as part of the value.
    // The log is duplicated: once in the value, once as the output.
    //
    // Time complexity: O(copy W)
    // Space complexity: O(size of A + 2 * size of W)
    template<Monoid W, typename A>
    constexpr Writer<W, std::pair<A, W>> listen(const Writer<W, A>& w) {
        auto [value, log] = w.runWriter();
        return Writer<W, std::pair<A, W>>(
            std::pair{std::move(value), log},
            std::move(log)
        );
    }

    // listen :: Writer w void → Writer w (int, w)
    // Specialization for Writer<W, void>
    // Returns Writer<W, std::pair<int, W>> where int represents void (0)
    template<Monoid W>
    constexpr Writer<W, std::pair<int, W>> listen(const Writer<W, void>& w) {
        auto [placeholder, log] = w.runWriter();  // placeholder is int (0)
        return Writer<W, std::pair<int, W>>(
            std::pair{placeholder, log},
            std::move(log)
        );
    }

    // pass :: Writer w (a, w → w) → Writer w a
    // pass (Writer (a, f) w) = Writer a (f w)
    //
    // Applies a function from the value to transform the log.
    // The value must be a pair of (result, log_transformer).
    //
    // Time complexity: O(f)
    // Space complexity: O(size of A + size of transformed W)
    template<Monoid W, typename A, typename F>
        requires std::invocable<F, W> && std::same_as<std::invoke_result_t<F, W>, W>
    constexpr Writer<W, A> pass(const Writer<W, std::pair<A, F>>& w) {
        auto [pair, log] = w.runWriter();
        auto [value, transform] = std::move(pair);
        return Writer<W, A>(std::move(value), transform(std::move(log)));
    }

    // censor :: (w → w) → Writer w a → Writer w a
    // censor f (Writer a w) = Writer a (f w)
    //
    // Transforms the log using the given function, preserving the value.
    // Useful for filtering or modifying log output.
    //
    // Time complexity: O(f)
    // Space complexity: O(size of A + size of transformed W)
    template<Monoid W, typename A, typename F>
        requires std::invocable<F, W> && std::same_as<std::invoke_result_t<F, W>, W>
    constexpr Writer<W, A> censor(F&& f, const Writer<W, A>& w) {
        auto [value, log] = w.runWriter();
        return Writer<W, A>(std::move(value), f(std::move(log)));
    }

    // ========================================================================
    // HELPER FUNCTIONS - Extracting Components
    // ========================================================================

    // execWriter :: Writer w a → w
    // execWriter (Writer a w) = w
    //
    // Runs the Writer and returns ONLY the accumulated log.
    // Discards the value (useful when only interested in output).
    //
    // Time complexity: O(copy W)
    // Space complexity: O(size of W)
    template<Monoid W, typename A>
    constexpr W execWriter(const Writer<W, A>& w) {
        auto [_, log] = w.runWriter();
        return log;
    }

    // evalWriter :: Writer w a → a
    // evalWriter (Writer a w) = a
    //
    // Runs the Writer and returns ONLY the value.
    // Discards the log (useful when log was for debugging/intermediate purposes).
    //
    // Time complexity: O(copy A)
    // Space complexity: O(size of A)
    template<Monoid W, typename A>
    constexpr A evalWriter(const Writer<W, A>& w) {
        auto [value, _] = w.runWriter();
        return value;
    }

    // mapWriter :: ((a, w) → (b, w)) → Writer w a → Writer w b
    // mapWriter f (Writer a w) = let (b, w') = f (a, w) in Writer b w'
    //
    // Transforms both the value and the log simultaneously.
    // Most general transformation function for Writer.
    //
    // Time complexity: O(f)
    // Space complexity: O(size of result)
    template<Monoid W, typename A, typename F>
        requires std::invocable<F, std::pair<A, W>>
    constexpr auto mapWriter(F&& f, const Writer<W, A>& w) {
        using Result = std::invoke_result_t<F, std::pair<A, W>>;
        using B = typename Result::first_type;

        auto pair = w.runWriter();
        auto [new_value, new_log] = f(std::move(pair));
        return Writer<W, B>(std::move(new_value), std::move(new_log));
    }

    // ========================================================================
    // USAGE PATTERNS AND EXAMPLES
    // ========================================================================
    //
    // 1. BASIC LOGGING:
    // ```cpp
    // auto computation = writer(42, std::string{"calculated value"});
    // auto [value, log] = computation.runWriter();
    // // value == 42, log == "calculated value"
    // ```
    //
    // 2. SEQUENTIAL LOGGING WITH BIND:
    // ```cpp
    // auto logged_add = [](int x, int y) {
    //     return writer(x + y, std::string{"add(" + std::to_string(x) + "," +
    //                                      std::to_string(y) + ")"});
    // };
    //
    // auto computation = bind(logged_add(5, 3), [](int sum) {
    //     return logged_add(sum, 2);
    // });
    //
    // auto [value, log] = computation.runWriter();
    // // value == 10, log == "add(5,3)add(8,2)"
    // ```
    //
    // 3. USING TELL FOR PURE LOGGING:
    // ```cpp
    // auto process = bind(tell(std::string{"Starting process"}), [](auto) {
    //     return bind(writer(42, std::string{" computed"}), [](int x) {
    //         return bind(tell(std::string{" finishing"}), [x](auto) {
    //             return pure<Writer, std::string>(x);
    //         });
    //     });
    // });
    //
    // auto [value, log] = process.runWriter();
    // // value == 42, log == "Starting process computed finishing"
    // ```
    //
    // 4. LOG TRANSFORMATION WITH CENSOR:
    // ```cpp
    // auto w = writer(42, std::string{"debug info"});
    // auto filtered = censor([](std::string s) {
    //     return s + " [FILTERED]";
    // }, w);
    //
    // auto [value, log] = filtered.runWriter();
    // // value == 42, log == "debug info [FILTERED]"
    // ```
    //
    // 5. ACCESSING LOG WITH LISTEN:
    // ```cpp
    // auto w = writer(10, std::string{"original"});
    // auto w_listened = listen(w);
    //
    // auto [pair, log] = w_listened.runWriter();
    // // pair.first == 10, pair.second == "original", log == "original"
    // ```
    //
    // 6. VECTOR ACCUMULATION:
    // ```cpp
    // auto trace = [](int x) {
    //     return writer(x * 2, std::vector<int>{x});
    // };
    //
    // auto computation = bind(trace(5), [trace](int x) {
    //     return trace(x);
    // });
    //
    // auto [value, log] = computation.runWriter();
    // // value == 20, log == {5, 10}
    // ```
    //
    // ========================================================================

} // namespace fp20
