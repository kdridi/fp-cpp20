// ============================================================================
// FP++20: ReaderT<E, M, A> Monad Transformer Implementation
// ============================================================================
// ReaderT adds environment/configuration access to any monad M.
// ReaderT e m a ≅ e → m a
//
// This allows composing computations that depend on a shared environment
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
// - Reference: [Haskell ReaderT] - Control.Monad.Trans.Reader
//   https://hackage.haskell.org/package/transformers/docs/Control-Monad-Trans-Reader.html
//   Standard Haskell implementation
//
// - Reference: [Modular Monad Transformers] - Liang, Hudak, Jones (1995)
//   POPL '95: Foundational paper on modular transformer design
//
// Dependency Injection:
// - Reference: [Reader Monad for Dependency Injection]
//   https://wiki.haskell.org/Dependency_injection
//   ReaderT as the functional DI solution
//
// ============================================================================
// READERT MONAD TRANSFORMER OVERVIEW
// ============================================================================
//
// KEY INSIGHT: ReaderT<E, M, A> = E → M<A>
//
// ReaderT layers environment access on top of any monad M:
// - All computations have access to a shared read-only environment E
// - Environment is threaded implicitly through bind operations
// - Combines dependency injection with the effects of the base monad M
//
// STRUCTURE:
// - runReaderT: Unwrap to get E → M<A>
// - lift: Lift M<A> to ReaderT<E, M, A> (ignores environment)
// - ask: Retrieve the environment
// - asks: Project a value from the environment
// - local: Run a computation with a modified environment
// - Functor/Applicative/Monad instances compose both layers
//
// TYPICAL USAGE:
// - ReaderT<Config, IO, Result>: IO operations with configuration
// - ReaderT<Logger, Either<Error>, Value>: Error handling with logging context
// - ReaderT<Database, StateT<Cache>, User>: Database access with caching
//
// ============================================================================
// MONAD LAWS FOR READERT
// ============================================================================
//
// Standard Monad Laws (for ReaderT<E, M, A> where M is a Monad):
// 1. Left Identity: return a >>= f ≡ f a
// 2. Right Identity: m >>= return ≡ m
// 3. Associativity: (m >>= f) >>= g ≡ m >>= (λx -> f x >>= g)
//
// Transformer-Specific Laws:
// 4. lift . return ≡ return
// 5. lift (m >>= f) ≡ lift m >>= (lift . f)
//
// Reader Laws:
// 6. ask >>= \e -> ask >>= \e' -> f e e' ≡ ask >>= \e -> f e e
// 7. local f ask ≡ fmap f ask
// 8. local f (local g m) ≡ local (g . f) m
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

    // ========================================================================
    // READERT TYPE - Environment/Reader Transformer
    // ========================================================================
    // ReaderT<E, M, A> represents a computation in monad M that depends on environment E.
    // Internally: E → M<A>
    //
    // The runReaderF_ member is the function from environment to monadic result.
    template<typename E, template<typename> class M, typename A>
    class ReaderT {
    public:
        using env_type = E;
        using value_type = A;
        using result_type = M<A>;

    private:
        std::function<result_type(E)> runReaderF_;

    public:
        // ====================================================================
        // CONSTRUCTORS
        // ====================================================================

        // Construct ReaderT from a function E → M<A>
        // Time complexity: O(1)
        // Space complexity: O(function object)
        explicit ReaderT(std::function<result_type(E)> f)
            : runReaderF_(std::move(f)) {}

        // Construct from any callable
        template<typename F>
            requires std::invocable<F, E> &&
                     std::same_as<std::invoke_result_t<F, E>, result_type>
        explicit ReaderT(F&& f)
            : runReaderF_(std::forward<F>(f)) {}

        // Copy/move semantics (defaulted)
        ReaderT(const ReaderT&) = default;
        ReaderT(ReaderT&&) noexcept = default;
        ReaderT& operator=(const ReaderT&) = default;
        ReaderT& operator=(ReaderT&&) noexcept = default;

        // ====================================================================
        // UNWRAPPING
        // ====================================================================

        // runReaderT :: ReaderT e m a -> e -> m a
        // Execute the Reader computation with the given environment.
        // Returns: M<A>
        //
        // Time complexity: O(computation)
        // Space complexity: O(M)
        [[nodiscard]] result_type runReaderT(E env) const {
            return runReaderF_(std::move(env));
        }
    };

    // ========================================================================
    // SMART CONSTRUCTORS
    // ========================================================================

    // readerT :: (e -> m a) -> ReaderT e m a
    // Wrap a function into ReaderT.
    //
    // Time complexity: O(1)
    // Space complexity: O(1)
    template<typename E, template<typename> class M, typename A>
    ReaderT<E, M, A> readerT(std::function<M<A>(E)> f) {
        return ReaderT<E, M, A>(std::move(f));
    }

    // ========================================================================
    // READER PRIMITIVES
    // ========================================================================

    // ask :: ReaderT e m e
    // Retrieve the entire environment as the result value.
    //
    // Time complexity: O(pure on M)
    // Space complexity: O(M)
    template<typename E, template<typename> class M>
        requires requires(E e) {
            { pure<M, E>(e) } -> std::convertible_to<M<E>>;
        }
    ReaderT<E, M, E> ask() {
        return ReaderT<E, M, E>([](E env) -> M<E> {
            return pure<M, E>(std::move(env));
        });
    }

    // asks :: (e -> a) -> ReaderT e m a
    // Project a value from the environment using a function.
    //
    // Time complexity: O(pure on M + f)
    // Space complexity: O(M)
    template<typename E, template<typename> class M, typename F>
        requires std::invocable<F, E>
    auto asks(F&& f) -> ReaderT<E, M, std::invoke_result_t<F, E>> {
        using A = std::invoke_result_t<F, E>;

        return ReaderT<E, M, A>([f = std::forward<F>(f)](E env) -> M<A> {
            return pure<M, A>(f(std::move(env)));
        });
    }

    // local :: (e -> e) -> ReaderT e m a -> ReaderT e m a
    // Execute a ReaderT computation with a modified environment.
    // The modification is local to this computation.
    //
    // Time complexity: O(computation)
    // Space complexity: O(M)
    template<typename E, template<typename> class M, typename A, typename F>
        requires std::invocable<F, E> &&
                 std::same_as<std::invoke_result_t<F, E>, E>
    ReaderT<E, M, A> local(F&& f, const ReaderT<E, M, A>& rt) {
        return ReaderT<E, M, A>([rt, f = std::forward<F>(f)](E env) -> M<A> {
            E modified_env = f(std::move(env));
            return rt.runReaderT(std::move(modified_env));
        });
    }

    // ========================================================================
    // LIFT OPERATION - The Key Transformer Function
    // ========================================================================

    // lift :: Monad m => m a -> ReaderT e m a
    // Lift a computation from the base monad M into ReaderT.
    // The environment is ignored.
    //
    // This is THE fundamental operation that makes ReaderT a transformer.
    //
    // Time complexity: O(1) to construct
    // Space complexity: O(M)
    template<typename E, template<typename> class M, typename A>
    ReaderT<E, M, A> lift(M<A> ma) {
        return ReaderT<E, M, A>([ma = std::move(ma)](E) -> M<A> {
            // Ignore the environment, just return the inner monad
            return ma;
        });
    }

    // ========================================================================
    // FUNCTOR INSTANCE - fmap over ReaderT
    // ========================================================================

    // fmap :: (a -> b) -> ReaderT e m a -> ReaderT e m b
    // Transform the result value, preserving environment access.
    //
    // Functor Laws:
    // 1. Identity: fmap(id, rt) ≡ rt
    // 2. Composition: fmap(g ∘ f, rt) ≡ fmap(g, fmap(f, rt))
    //
    // Time complexity: O(fmap on M)
    // Space complexity: O(result)
    template<typename Func, typename E, template<typename> class M, typename A>
        requires std::invocable<Func, A>
    auto fmap(Func&& f, const ReaderT<E, M, A>& rt) {
        using B = std::invoke_result_t<Func, A>;

        return ReaderT<E, M, B>([rt, f = std::forward<Func>(f)](E env) -> M<B> {
            // Run the inner ReaderT and map over the result in M
            auto m_a = rt.runReaderT(env);
            return fmap(f, std::move(m_a));
        });
    }

    // ========================================================================
    // APPLICATIVE INSTANCE
    // ========================================================================

    // pure :: a -> ReaderT e m a
    // Lift a pure value into ReaderT.
    // Environment is ignored.
    //
    // Time complexity: O(pure on M)
    // Space complexity: O(M)
    template<template<typename> class ReaderT_T, typename E, template<typename> class M, typename A>
        requires std::same_as<ReaderT_T<A>, ReaderT<E, M, A>> &&
                 requires(A a) {
                     { pure<M, A>(a) } -> std::convertible_to<M<A>>;
                 }
    ReaderT<E, M, A> pure(A value) {
        return ReaderT<E, M, A>([value = std::move(value)](E) -> M<A> {
            return pure<M, A>(value);
        });
    }

    // apply :: ReaderT e m (a -> b) -> ReaderT e m a -> ReaderT e m b
    // Apply a function in ReaderT context to a value in ReaderT context.
    // Both computations share the same environment.
    //
    // Time complexity: O(bind on M)
    // Space complexity: O(result)
    template<typename E, template<typename> class M, typename Func, typename A>
        requires std::invocable<Func, A>
    auto apply(const ReaderT<E, M, Func>& rtf, const ReaderT<E, M, A>& rta) {
        using B = std::invoke_result_t<Func, A>;

        return ReaderT<E, M, B>([rtf, rta](E env) -> M<B> {
            // Run both with the same environment
            auto m_f = rtf.runReaderT(env);
            auto m_a = rta.runReaderT(env);

            // Bind in M to apply the function
            return bind(
                std::move(m_f),
                [m_a = std::move(m_a)](Func f) -> M<B> {
                    return fmap(
                        [f = std::move(f)](A a) -> B {
                            return f(std::move(a));
                        },
                        m_a
                    );
                }
            );
        });
    }

    // ========================================================================
    // MONAD INSTANCE
    // ========================================================================

    // bind :: ReaderT e m a -> (a -> ReaderT e m b) -> ReaderT e m b
    // Sequences ReaderT computations, threading the same environment through both.
    //
    // The key operation for ReaderT:
    // 1. Run m with environment e to get M<A>
    // 2. In the monad M, bind to extract A
    // 3. Apply k to A to get ReaderT<E, M, B>
    // 4. Run that with the same environment e to get M<B>
    //
    // Monad Laws:
    // 1. Left Identity: bind(pure(a), k) ≡ k(a)
    // 2. Right Identity: bind(m, pure) ≡ m
    // 3. Associativity: bind(bind(m, f), g) ≡ bind(m, [](auto x) { return bind(f(x), g); })
    //
    // Time complexity: O(bind on M)
    // Space complexity: O(result)
    template<typename E, template<typename> class M, typename A, typename Func>
        requires std::invocable<Func, A> &&
                 requires(Func f, A a) {
                     { f(a) } -> std::convertible_to<ReaderT<E, M, typename std::invoke_result_t<Func, A>::value_type>>;
                 }
    auto bind(const ReaderT<E, M, A>& m, Func&& k) {
        using ReaderTB = std::invoke_result_t<Func, A>;
        using B = typename ReaderTB::value_type;

        return ReaderT<E, M, B>([m, k = std::forward<Func>(k)](E env) -> M<B> {
            // Run the first computation with the environment
            auto m_a = m.runReaderT(env);

            // Bind in the inner monad
            return bind(
                std::move(m_a),
                [k, env](A a) -> M<B> {
                    // Apply continuation to get new ReaderT
                    ReaderTB rtb = k(std::move(a));

                    // Run it with the same environment
                    return rtb.runReaderT(env);
                }
            );
        });
    }

    // return_ :: a -> ReaderT e m a
    // Alias for pure (monadic return).
    template<template<typename> class ReaderT_T, typename E, template<typename> class M, typename A>
        requires std::same_as<ReaderT_T<A>, ReaderT<E, M, A>>
    ReaderT<E, M, A> return_(A value) {
        return pure<ReaderT_T, E, M, A>(std::move(value));
    }

    // ========================================================================
    // HELPER FUNCTIONS
    // ========================================================================

    // withReaderT :: (e' -> e) -> ReaderT e m a -> ReaderT e' m a
    // Transform the environment before running the computation.
    //
    // Time complexity: O(computation)
    // Space complexity: O(result)
    template<typename E, typename E_new, template<typename> class M, typename A, typename F>
        requires std::invocable<F, E_new> &&
                 std::same_as<std::invoke_result_t<F, E_new>, E>
    ReaderT<E_new, M, A> withReaderT(F&& f, const ReaderT<E, M, A>& rt) {
        return ReaderT<E_new, M, A>([rt, f = std::forward<F>(f)](E_new env_new) -> M<A> {
            E env = f(std::move(env_new));
            return rt.runReaderT(std::move(env));
        });
    }

    // mapReaderT :: (m a -> n b) -> ReaderT e m a -> ReaderT e n b
    // Transform the inner monad computation.
    //
    // Time complexity: O(f)
    // Space complexity: O(result)
    template<typename E, template<typename> class M, template<typename> class N, typename A, typename B, typename Func>
        requires std::invocable<Func, M<A>> &&
                 std::convertible_to<std::invoke_result_t<Func, M<A>>, N<B>>
    ReaderT<E, N, B> mapReaderT(Func&& f, const ReaderT<E, M, A>& rt) {
        return ReaderT<E, N, B>([rt, f = std::forward<Func>(f)](E env) -> N<B> {
            return f(rt.runReaderT(std::move(env)));
        });
    }

    // ========================================================================
    // USAGE PATTERNS AND EXAMPLES
    // ========================================================================
    //
    // 1. BASIC READERT WITH IO:
    // ```cpp
    // struct Config {
    //     std::string db_url;
    //     int timeout;
    // };
    //
    // using App = ReaderT<Config, IO, std::string>;
    //
    // auto get_db_url = asks<Config, IO>([](const Config& cfg) {
    //     return cfg.db_url;
    // });
    //
    // auto connect = bind(
    //     get_db_url,
    //     [](std::string url) {
    //         return lift<Config, IO>(putStrLn("Connecting to: " + url));
    //     }
    // );
    //
    // Config cfg{"localhost:5432", 30};
    // auto io_action = connect.runReaderT(cfg);
    // io_action.unsafeRun();
    // ```
    //
    // 2. LOCAL ENVIRONMENT MODIFICATION:
    // ```cpp
    // auto with_debug_mode = [](App action) {
    //     return local(
    //         [](Config cfg) {
    //             cfg.debug = true;
    //             return cfg;
    //         },
    //         action
    //     );
    // };
    //
    // auto production = get_config();
    // auto debug = with_debug_mode(get_config());
    // // debug sees modified config, production sees original
    // ```
    //
    // 3. DEPENDENCY INJECTION:
    // ```cpp
    // using LoggerApp = ReaderT<Logger, IO, Unit>;
    //
    // auto log_message = [](std::string msg) -> LoggerApp {
    //     return bind(
    //         ask<Logger, IO>(),
    //         [msg](Logger logger) {
    //             return lift<Logger, IO>(logger.write(msg));
    //         }
    //     );
    // };
    //
    // auto business_logic = bind(
    //     log_message("Starting"),
    //     [](Unit) {
    //         return bind(
    //             do_work(),
    //             [](Result r) {
    //                 return log_message("Done: " + r.toString());
    //             }
    //         );
    //     }
    // );
    //
    // Logger file_logger = create_file_logger("app.log");
    // business_logic.runReaderT(file_logger).unsafeRun();
    // ```
    //
    // 4. STACKING WITH OTHER TRANSFORMERS:
    // ```cpp
    // // ReaderT<Config, EitherT<IO, Error>, User>
    // using App = ReaderT<Config, EitherT<IO, std::string>, User>;
    //
    // auto authenticate = bind(
    //     asks<Config, EitherT<IO, std::string>>([](const Config& c) { return c.auth_token; }),
    //     [](std::string token) {
    //         return lift<Config, EitherT<IO, std::string>>(
    //             validate_token(token)
    //         );
    //     }
    // );
    // ```
    //
    // ========================================================================

} // namespace fp20
