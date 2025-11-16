// ============================================================================
// FP++20: WriterT<W, M, A> Monad Transformer Implementation
// ============================================================================
// WriterT adds logging/accumulation semantics to any monad M.
// WriterT w m a ≅ m (a, w) where w is a Monoid
//
// This allows composing computations that produce output/logs
// with other effects like IO, State, etc.
//
// ============================================================================
// ACADEMIC REFERENCES & FOUNDATIONAL LITERATURE
// ============================================================================
//
// Monad Transformers:
// - Reference: [Monad Transformers Step by Step] - Martin Grabmüller (2006)
//   https://page.mi.fu-berlin.de/scravy/realworldhaskell/materialien/monad-transformers-step-by-step.pdf
//
// - Reference: [Haskell WriterT] - Control.Monad.Trans.Writer
//   https://hackage.haskell.org/package/transformers/docs/Control-Monad-Trans-Writer-Strict.html
//   Standard Haskell implementation
//
// - Reference: [Modular Monad Transformers] - Liang, Hudak, Jones (1995)
//   POPL '95: Foundational paper on modular transformer design
//
// Logging and Accumulation:
// - Reference: [Writer Monad for Logging]
//   https://wiki.haskell.org/All_About_Monads#The_Writer_monad
//
// ============================================================================
// WRITERT MONAD TRANSFORMER OVERVIEW
// ============================================================================
//
// KEY INSIGHT: WriterT<W, M, A> = M<(A, W)> where W is a Monoid
//
// WriterT layers logging/accumulation on top of any monad M:
// - Each computation produces a value A and accumulates output W
// - Output is combined using the monoid operation (mappend)
// - Combines logging with the effects of the base monad M
//
// STRUCTURE:
// - runWriterT: Unwrap to get M<(A, W)>
// - lift: Lift M<A> to WriterT<W, M, A> with empty log (mempty)
// - tell: Append to the log
// - listen: Expose the accumulated log
// - censor: Transform the log
// - Functor/Applicative/Monad instances compose both layers
//
// TYPICAL USAGE:
// - WriterT<std::string, IO, Result>: IO operations with logging
// - WriterT<std::vector<Event>, State, Value>: Stateful computations with event log
// - WriterT<Metrics, Either<Error>, Data>: Error handling with metrics
//
// ============================================================================
// MONAD LAWS FOR WRITERT
// ============================================================================
//
// Standard Monad Laws (for WriterT<W, M, A> where M is a Monad, W is a Monoid):
// 1. Left Identity: return a >>= f ≡ f a
// 2. Right Identity: m >>= return ≡ m
// 3. Associativity: (m >>= f) >>= g ≡ m >>= (λx -> f x >>= g)
//
// Transformer-Specific Laws:
// 4. lift . return ≡ return
// 5. lift (m >>= f) ≡ lift m >>= (lift . f)
//
// Writer Laws:
// 6. tell w1 >> tell w2 ≡ tell (w1 <> w2)
// 7. listen (tell w) ≡ tell w >> return ((), w)
// 8. listen (return a) ≡ return (a, mempty)
//
// ============================================================================

#pragma once

#include <fp20/concepts/monoid.hpp>
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
    // WRITERT TYPE - Writer/Logging Transformer
    // ========================================================================
    // WriterT<W, M, A> represents a computation in monad M that produces output W.
    // Internally: M<(A, W)>
    //
    // The value_ member is the inner monad containing a pair of (value, log).
    template<Monoid W, template<typename> class M, typename A>
    class WriterT {
    public:
        using log_type = W;
        using value_type = A;
        using inner_monad_type = M<std::pair<A, W>>;

    private:
        inner_monad_type value_;

    public:
        // ====================================================================
        // CONSTRUCTORS
        // ====================================================================

        // Construct WriterT from the inner monad M<(A, W)>
        // Time complexity: O(1) + O(M constructor)
        // Space complexity: O(size of M<(A, W)>)
        explicit WriterT(inner_monad_type value)
            : value_(std::move(value)) {}

        // Copy/move semantics (defaulted)
        WriterT(const WriterT&) = default;
        WriterT(WriterT&&) noexcept = default;
        WriterT& operator=(const WriterT&) = default;
        WriterT& operator=(WriterT&&) noexcept = default;

        // ====================================================================
        // UNWRAPPING
        // ====================================================================

        // runWriterT :: WriterT w m a -> m (a, w)
        // Extract the inner monad computation.
        // Returns: M<(A, W)>
        //
        // Time complexity: O(copy/move M)
        // Space complexity: O(size of M)
        [[nodiscard]] inner_monad_type runWriterT() const {
            return value_;
        }

        // Non-const version for move extraction
        [[nodiscard]] inner_monad_type runWriterT() {
            return std::move(value_);
        }
    };

    // ========================================================================
    // SMART CONSTRUCTORS
    // ========================================================================

    // writerT :: m (a, w) -> WriterT w m a
    // Wrap an M<(A, W)> into WriterT.
    //
    // Time complexity: O(1)
    // Space complexity: O(1)
    template<Monoid W, template<typename> class M, typename A>
    WriterT<W, M, A> writerT(M<std::pair<A, W>> inner) {
        return WriterT<W, M, A>(std::move(inner));
    }

    // writer :: a -> w -> WriterT w m a
    // Construct a WriterT with a value and log.
    //
    // Time complexity: O(pure on M)
    // Space complexity: O(M + A + W)
    template<Monoid W, template<typename> class M, typename A>
        requires requires(A a, W w) {
            { pure<M, std::pair<A, W>>(std::pair<A, W>(a, w)) } -> std::convertible_to<M<std::pair<A, W>>>;
        }
    WriterT<W, M, A> writer(A value, W log) {
        return WriterT<W, M, A>(pure<M, std::pair<A, W>>(std::pair<A, W>(std::move(value), std::move(log))));
    }

    // ========================================================================
    // WRITER PRIMITIVES
    // ========================================================================

    // tell :: w -> WriterT w m Unit
    // Append to the log without producing a meaningful value.
    //
    // Time complexity: O(pure on M)
    // Space complexity: O(M + W)
    template<Monoid W, template<typename> class M>
        requires requires(W w) {
            { pure<M, std::pair<Unit, W>>(std::pair<Unit, W>(Unit{}, w)) } -> std::convertible_to<M<std::pair<Unit, W>>>;
        }
    WriterT<W, M, Unit> tell(W log) {
        return WriterT<W, M, Unit>(pure<M, std::pair<Unit, W>>(std::pair<Unit, W>(Unit{}, std::move(log))));
    }

    // listen :: WriterT w m a -> WriterT w m (a, w)
    // Expose the accumulated log as part of the value.
    //
    // Time complexity: O(fmap on M)
    // Space complexity: O(M)
    template<Monoid W, template<typename> class M, typename A>
    WriterT<W, M, std::pair<A, W>> listen(const WriterT<W, M, A>& wt) {
        auto mapped = fmap(
            [](std::pair<A, W> p) -> std::pair<std::pair<A, W>, W> {
                auto [a, w] = std::move(p);
                return std::pair<std::pair<A, W>, W>(std::pair<A, W>(a, w), w);
            },
            wt.runWriterT()
        );

        return WriterT<W, M, std::pair<A, W>>(std::move(mapped));
    }

    // censor :: (w -> w) -> WriterT w m a -> WriterT w m a
    // Transform the log using a function.
    //
    // Time complexity: O(fmap on M + f)
    // Space complexity: O(result)
    template<Monoid W, template<typename> class M, typename A, typename F>
        requires std::invocable<F, W> &&
                 std::same_as<std::invoke_result_t<F, W>, W>
    WriterT<W, M, A> censor(F&& f, const WriterT<W, M, A>& wt) {
        auto mapped = fmap(
            [f = std::forward<F>(f)](std::pair<A, W> p) -> std::pair<A, W> {
                auto [a, w] = std::move(p);
                return std::pair<A, W>(std::move(a), f(std::move(w)));
            },
            wt.runWriterT()
        );

        return WriterT<W, M, A>(std::move(mapped));
    }

    // ========================================================================
    // LIFT OPERATION - The Key Transformer Function
    // ========================================================================

    // lift :: Monad m => m a -> WriterT w m a
    // Lift a computation from the base monad M into WriterT.
    // The log is initialized to mempty (empty).
    //
    // This is THE fundamental operation that makes WriterT a transformer.
    //
    // Time complexity: O(fmap on M)
    // Space complexity: O(M)
    template<Monoid W, template<typename> class M, typename A>
        requires requires(M<A> ma) {
            { fmap([](A a) { return std::pair<A, W>(a, mempty<W>()); }, ma) } -> std::convertible_to<M<std::pair<A, W>>>;
        }
    WriterT<W, M, A> lift(M<A> ma) {
        // Lift by pairing the value with empty log
        auto wrapped = fmap([](A a) { return std::pair<A, W>(std::move(a), mempty<W>()); }, std::move(ma));
        return WriterT<W, M, A>(std::move(wrapped));
    }

    // ========================================================================
    // FUNCTOR INSTANCE - fmap over WriterT
    // ========================================================================

    // fmap :: (a -> b) -> WriterT w m a -> WriterT w m b
    // Transform the value, preserving the log.
    //
    // Functor Laws:
    // 1. Identity: fmap(id, wt) ≡ wt
    // 2. Composition: fmap(g ∘ f, wt) ≡ fmap(g, fmap(f, wt))
    //
    // Time complexity: O(fmap on M)
    // Space complexity: O(result)
    template<typename Func, Monoid W, template<typename> class M, typename A>
        requires std::invocable<Func, A>
    auto fmap(Func&& f, const WriterT<W, M, A>& wt) {
        using B = std::invoke_result_t<Func, A>;

        auto mapped = fmap(
            [f = std::forward<Func>(f)](std::pair<A, W> p) -> std::pair<B, W> {
                auto [a, w] = std::move(p);
                return std::pair<B, W>(f(std::move(a)), std::move(w));
            },
            wt.runWriterT()
        );

        return WriterT<W, M, B>(std::move(mapped));
    }

    // ========================================================================
    // APPLICATIVE INSTANCE
    // ========================================================================

    // pure :: a -> WriterT w m a
    // Lift a pure value into WriterT.
    // Log is initialized to mempty.
    //
    // Time complexity: O(pure on M)
    // Space complexity: O(M)
    template<template<typename> class WriterT_T, Monoid W, template<typename> class M, typename A>
        requires std::same_as<WriterT_T<A>, WriterT<W, M, A>> &&
                 requires(A a) {
                     { pure<M, std::pair<A, W>>(std::pair<A, W>(a, mempty<W>())) } -> std::convertible_to<M<std::pair<A, W>>>;
                 }
    WriterT<W, M, A> pure(A value) {
        return WriterT<W, M, A>(pure<M, std::pair<A, W>>(std::pair<A, W>(std::move(value), mempty<W>())));
    }

    // apply :: WriterT w m (a -> b) -> WriterT w m a -> WriterT w m b
    // Apply a function in WriterT context to a value in WriterT context.
    // Logs are concatenated using mappend.
    //
    // Time complexity: O(bind on M + mappend)
    // Space complexity: O(result)
    template<Monoid W, template<typename> class M, typename Func, typename A>
        requires std::invocable<Func, A>
    auto apply(const WriterT<W, M, Func>& wtf, const WriterT<W, M, A>& wta) {
        using B = std::invoke_result_t<Func, A>;

        auto result = bind(
            wtf.runWriterT(),
            [wta_inner = wta.runWriterT()](std::pair<Func, W> p_f) -> M<std::pair<B, W>> {
                auto [f, w1] = std::move(p_f);

                return bind(
                    wta_inner,
                    [f = std::move(f), w1 = std::move(w1)](std::pair<A, W> p_a) -> M<std::pair<B, W>> {
                        auto [a, w2] = std::move(p_a);

                        return pure<M, std::pair<B, W>>(
                            std::pair<B, W>(f(std::move(a)), mappend(std::move(w1), std::move(w2)))
                        );
                    }
                );
            }
        );

        return WriterT<W, M, B>(std::move(result));
    }

    // ========================================================================
    // MONAD INSTANCE
    // ========================================================================

    // bind :: WriterT w m a -> (a -> WriterT w m b) -> WriterT w m b
    // Sequences WriterT computations, concatenating logs.
    //
    // The key operation for WriterT:
    // 1. Run m to get M<(a, w1)>
    // 2. In the monad M, extract (a, w1)
    // 3. Apply k to a to get WriterT<W, M, B>
    // 4. Run that to get M<(b, w2)>
    // 5. Combine logs: w1 <> w2
    //
    // Monad Laws:
    // 1. Left Identity: bind(pure(a), k) ≡ k(a)
    // 2. Right Identity: bind(m, pure) ≡ m
    // 3. Associativity: bind(bind(m, f), g) ≡ bind(m, [](auto x) { return bind(f(x), g); })
    //
    // Time complexity: O(bind on M + mappend)
    // Space complexity: O(result)
    template<Monoid W, template<typename> class M, typename A, typename Func>
        requires std::invocable<Func, A> &&
                 requires(Func f, A a) {
                     { f(a) } -> std::convertible_to<WriterT<W, M, typename std::invoke_result_t<Func, A>::value_type>>;
                 }
    auto bind(const WriterT<W, M, A>& m, Func&& k) {
        using WriterTB = std::invoke_result_t<Func, A>;
        using B = typename WriterTB::value_type;

        auto result = bind(
            m.runWriterT(),
            [k = std::forward<Func>(k)](std::pair<A, W> p) -> M<std::pair<B, W>> {
                auto [a, w1] = std::move(p);

                // Apply continuation to get WriterT
                WriterTB wtb = k(std::move(a));

                // Run it and combine logs
                auto m_pair_b = wtb.runWriterT();

                return fmap(
                    [w1 = std::move(w1)](std::pair<B, W> p_b) -> std::pair<B, W> {
                        auto [b, w2] = std::move(p_b);
                        return std::pair<B, W>(std::move(b), mappend(w1, std::move(w2)));
                    },
                    std::move(m_pair_b)
                );
            }
        );

        return WriterT<W, M, B>(std::move(result));
    }

    // return_ :: a -> WriterT w m a
    // Alias for pure (monadic return).
    template<template<typename> class WriterT_T, Monoid W, template<typename> class M, typename A>
        requires std::same_as<WriterT_T<A>, WriterT<W, M, A>>
    WriterT<W, M, A> return_(A value) {
        return pure<WriterT_T, W, M, A>(std::move(value));
    }

    // ========================================================================
    // HELPER FUNCTIONS
    // ========================================================================

    // execWriterT :: WriterT w m a -> m w
    // Run WriterT and return only the log (discard the value).
    //
    // Time complexity: O(fmap on M)
    // Space complexity: O(M)
    template<Monoid W, template<typename> class M, typename A>
    M<W> execWriterT(const WriterT<W, M, A>& wt) {
        return fmap(
            [](std::pair<A, W> p) -> W {
                return std::move(p.second);
            },
            wt.runWriterT()
        );
    }

    // mapWriterT :: (m (a, w) -> n (b, w')) -> WriterT w m a -> WriterT w' n b
    // Transform the inner monad computation.
    //
    // Time complexity: O(f)
    // Space complexity: O(result)
    template<Monoid W, Monoid W_new, template<typename> class M, template<typename> class N, typename A, typename B, typename Func>
        requires std::invocable<Func, M<std::pair<A, W>>> &&
                 std::convertible_to<std::invoke_result_t<Func, M<std::pair<A, W>>>, N<std::pair<B, W_new>>>
    WriterT<W_new, N, B> mapWriterT(Func&& f, const WriterT<W, M, A>& wt) {
        return WriterT<W_new, N, B>(f(wt.runWriterT()));
    }

    // ========================================================================
    // USAGE PATTERNS AND EXAMPLES
    // ========================================================================
    //
    // 1. BASIC WRITERT WITH IO:
    // ```cpp
    // using LogIO = WriterT<std::string, IO, int>;
    //
    // auto compute_with_log = bind(
    //     tell<std::string, IO>("Starting computation\n"),
    //     [](Unit) {
    //         return bind(
    //             writer<std::string, IO>(42, "Computed value\n"),
    //             [](int x) {
    //                 return writer<std::string, IO>(x * 2, "Doubled value\n");
    //             }
    //         );
    //     }
    // );
    //
    // auto io_action = compute_with_log.runWriterT();
    // auto [value, log] = io_action.unsafeRun();
    // // value == 84
    // // log == "Starting computation\nComputed value\nDoubled value\n"
    // ```
    //
    // 2. LIFTING IO INTO WRITERT:
    // ```cpp
    // IO<int> read_input = effect([]{ return 42; });
    // WriterT<std::string, IO, int> logged = lift<std::string, IO>(std::move(read_input));
    // // Empty log initially
    //
    // auto with_log = bind(
    //     logged,
    //     [](int x) {
    //         return tell<std::string, IO>("Read: " + std::to_string(x) + "\n");
    //     }
    // );
    // ```
    //
    // 3. LISTEN TO ACCUMULATED LOG:
    // ```cpp
    // auto computation = writer<std::string, IO>(100, "initial\n");
    // auto with_listen = listen(computation);
    //
    // auto io_action = with_listen.runWriterT();
    // auto [pair, log] = io_action.unsafeRun();
    // // pair.first == 100
    // // pair.second == "initial\n"
    // // log == "initial\n"
    // ```
    //
    // 4. CENSOR TO FILTER LOGS:
    // ```cpp
    // auto verbose = writer<std::string, IO>(42, "DEBUG: value computed\n");
    // auto filtered = censor(
    //     [](std::string s) {
    //         return s.find("DEBUG") != std::string::npos ? "" : s;
    //     },
    //     verbose
    // );
    // // Log is filtered out if it contains "DEBUG"
    // ```
    //
    // 5. VECTOR ACCUMULATION:
    // ```cpp
    // using EventLog = WriterT<std::vector<Event>, IO, Result>;
    //
    // auto process = bind(
    //     tell<std::vector<Event>, IO>({Event{"Started"}}),
    //     [](Unit) {
    //         return bind(
    //             do_work(),
    //             [](Result r) {
    //                 return tell<std::vector<Event>, IO>({Event{"Finished"}});
    //             }
    //         );
    //     }
    // );
    //
    // auto io_action = process.runWriterT();
    // auto [result, events] = io_action.unsafeRun();
    // // events == {Event{"Started"}, Event{"Finished"}}
    // ```
    //
    // ========================================================================

} // namespace fp20
