// ============================================================================
// FP++20: IO<A> Monad Implementation
// ============================================================================
// The IO monad encapsulates side-effecting computations with lazy evaluation,
// providing referential transparency through delayed execution. IO represents
// a description of an effectful computation rather than executing it directly.
//
// ============================================================================
// ACADEMIC REFERENCES & FOUNDATIONAL LITERATURE
// ============================================================================
//
// Foundational Papers:
// - Reference: [Tackling the Awkward Squad] - Simon Peyton Jones (2001)
//   https://www.microsoft.com/en-us/research/publication/tackling-awkward-squad-monadic-inputoutput-concurrency-exceptions-foreign-language-calls-haskell/
//   The seminal paper on IO monad design in Haskell
//
// - Reference: [Imperative Functional Programming] - Simon Peyton Jones & Philip Wadler (1993)
//   https://www.microsoft.com/en-us/research/publication/imperative-functional-programming/
//   Early approach to IO in functional languages
//
// - Reference: [Notions of Computation and Monads] - Eugenio Moggi (1991)
//   Information and Computation 93(1)
//   Foundational paper establishing monads for computational effects
//
// - Reference: [Monads for Functional Programming] - Philip Wadler (1995)
//   Advanced Functional Programming, Springer LNCS 925
//   Section on IO monad and side effects
//
// Haskell Implementation:
// - Reference: [Haskell IO Monad] - https://wiki.haskell.org/IO_inside
//   Internal representation and semantics of Haskell's IO
//
// - Reference: [Haskell 2010 Report: IO]
//   https://www.haskell.org/onlinereport/haskell2010/haskellch7.html
//   Official specification of IO monad behavior
//
// Category Theory:
// - Reference: [Monads and Effects] - https://ncatlab.org/nlab/show/monad+%28in+computer+science%29
//   Categorical foundations of effect systems
//
// - Reference: [Category Theory for Programmers] - Bartosz Milewski
//   https://bartoszmilewski.com/2014/12/23/kleisli-categories/
//   Kleisli categories and monadic composition
//
// Design Patterns:
// - Reference: [Functional Programming in Scala] - Chiusano & Bjarnason
//   Chapter 13: External Effects and I/O
//   Practical approach to IO monad implementation
//
// ============================================================================
// IO MONAD OVERVIEW
// ============================================================================
//
// KEY INSIGHT: IO<A> represents a "description" of a computation that,
// when executed, will perform side effects and produce a value of type A.
//
// CRITICAL PROPERTY: Lazy Evaluation
// - IO values are PURE - they are descriptions, not executions
// - No effects occur until unsafeRun() is called
// - Multiple references to same IO value won't cause multiple executions
// - This preserves referential transparency in the pure subset of code
//
// SEMANTIC MODEL: IO<A> ≅ RealWorld → (A, RealWorld)
// - Similar to State monad but with "RealWorld" as the state
// - In practice: IO<A> ≅ () → A (thunk/suspension)
// - Captures the essence of sequential, effectful computation
//
// ============================================================================
// IO MONAD DESIGN PHILOSOPHY
// ============================================================================
//
// 1. REFERENTIAL TRANSPARENCY:
//    - Pure code cannot execute IO actions directly
//    - IO actions can only be composed, not run
//    - Only the "main" function (or test framework) calls unsafeRun()
//
// 2. COMPOSITION OVER EXECUTION:
//    - Build complex IO programs by composing simple IO actions
//    - Monadic bind (>>=) sequences effects
//    - Effects remain descriptions until the edge of the program
//
// 3. TYPE-LEVEL EFFECT TRACKING:
//    - Functions returning IO<A> explicitly declare they perform effects
//    - Pure functions return plain A, effectful ones return IO<A>
//    - Type system enforces separation of pure and impure code
//
// 4. LAZY EVALUATION:
//    - IO values are cheap to create (just function objects)
//    - No work happens until unsafeRun()
//    - Enables powerful abstractions (infinite IO sequences, conditional execution)
//
// ============================================================================
// MONAD LAWS FOR IO
// ============================================================================
//
// Standard Monad Laws (must hold for all IO<A>):
// 1. Left Identity: return a >>= f ≡ f a
//    pure(x) >>= f  ==  f(x)
//
// 2. Right Identity: m >>= return ≡ m
//    m >>= pure  ==  m
//
// 3. Associativity: (m >>= f) >>= g ≡ m >>= (λx -> f x >>= g)
//    bind(bind(m, f), g)  ==  bind(m, [](auto x) { return bind(f(x), g); })
//
// IO-Specific Properties:
// 4. Lazy Evaluation: Constructing IO<A> performs NO effects
//    auto action = putStrLn("Hello");  // Nothing printed yet!
//
// 5. Sequential Execution: bind enforces left-to-right sequencing
//    bind(action1, [](auto) { return action2; })  // action1 THEN action2
//
// 6. Referential Transparency of Descriptions:
//    auto io = putStrLn("Hello");
//    bind(io, [&](auto) { return io; })  // Same description, different executions
//
// ============================================================================

#pragma once

#include <functional>
#include <type_traits>
#include <utility>
#include <concepts>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdexcept>

namespace fp20 {

    // ========================================================================
    // UNIT TYPE FOR VOID-LIKE EFFECTS
    // ========================================================================
    // Many IO actions don't produce meaningful values (e.g., putStrLn, file writes).
    // We use Unit as a value-level representation of "no meaningful result".
    // This is equivalent to () in Haskell.
    struct Unit {
        constexpr bool operator==(const Unit&) const noexcept = default;
    };

    // ========================================================================
    // IO<A> TYPE - Suspended Effectful Computation
    // ========================================================================
    // IO<A> encapsulates a computation () → A that performs side effects.
    // The computation is SUSPENDED until unsafeRun() is called.
    //
    // Design choices:
    // - std::function provides type erasure and composability
    // - explicit constructor prevents accidental conversion
    // - const methods maintain purity of the description
    template<typename A>
    class IO {
    public:
        using value_type = A;

    private:
        // The suspended computation: a thunk that produces A when executed
        // Invariant: Must be callable with no arguments, returning A
        std::function<A()> computation_;

    public:
        // ====================================================================
        // CONSTRUCTORS
        // ====================================================================

        // Construct IO from a suspended computation (thunk)
        // The computation is NOT executed; it's merely stored.
        //
        // Example:
        //   IO<int> action([](){
        //       std::cout << "Effect!";
        //       return 42;
        //   });
        //   // Nothing printed yet - computation is suspended
        explicit IO(std::function<A()> comp)
            : computation_(std::move(comp)) {}

        // Construct from any callable (lambda, function object)
        // Uses C++20 concepts for type-safe perfect forwarding
        template<typename F>
            requires std::invocable<F> &&
                     std::same_as<std::invoke_result_t<F>, A>
        explicit IO(F&& f)
            : computation_(std::forward<F>(f)) {}

        // Move-only semantics
        // IO actions should execute exactly once, so we disable copying
        // This enforces that effects are controlled and explicit
        IO(const IO&) = delete;
        IO(IO&&) noexcept = default;
        IO& operator=(const IO&) = delete;
        IO& operator=(IO&&) noexcept = default;

        // ====================================================================
        // UNSAFE EXECUTION - The Edge of Purity
        // ====================================================================

        // unsafeRun :: IO A → A
        //
        // EXECUTES the suspended computation, performing side effects.
        // This is the ONLY way to "run" an IO action.
        //
        // WHY "UNSAFE"?
        // - Breaks referential transparency (effects modify real world)
        // - Should only be called at program boundaries (main, tests)
        // - Makes effect execution explicit and visible in code
        //
        // WHY NON-CONST?
        // - Executing effects mutates the "world state"
        // - Non-const enforces that IO is moved/consumed when run
        // - Prevents accidental multiple executions via copies
        //
        // SEMANTICS:
        // - Executes computation_ exactly once
        // - Returns the result value
        // - Any side effects occur during this call
        // - Consumes the IO action (move-only, so can't run again)
        //
        // Example:
        //   IO<int> action = effect([]{ return 42; });
        //   int x = std::move(action).unsafeRun();  // NOW the effect happens
        //   // action is now moved-from, can't run again
        [[nodiscard]] A unsafeRun() const {
            return computation_();
        }
    };

    // ========================================================================
    // SMART CONSTRUCTORS - Building IO Actions
    // ========================================================================

    // pure :: A → IO A
    //
    // Lifts a pure value into IO context with no side effects.
    // The resulting IO<A> is a trivial computation that just returns the value.
    //
    // Category theory: This is the unit/return of the IO monad (η).
    //
    // Example:
    //   IO<int> action = pure<IO>(42);
    //   action.unsafeRun();  // Returns 42, no effects
    //
    // Time complexity: O(1)
    // Space complexity: O(1)
    template<template<typename...> typename M, typename A>
        requires std::same_as<M<A>, IO<A>>
    IO<A> pure(A value) {
        return IO<A>([v = std::move(value)]() -> A {
            return v;
        });
    }

    // effect :: (() → A) → IO A
    //
    // Wraps an effectful computation as an IO action.
    // The computation is SUSPENDED - no effects occur until unsafeRun().
    //
    // This is the primary way to introduce new IO primitives.
    //
    // Example:
    //   auto readFile = effect([]{
    //       return std::filesystem::read("data.txt");
    //   });
    //   // File NOT read yet - only a description
    //
    // Time complexity: O(1) to construct
    // Space complexity: O(1)
    template<typename F>
        requires std::invocable<F>
    auto effect(F&& f) -> IO<std::invoke_result_t<F>> {
        using A = std::invoke_result_t<F>;
        return IO<A>(std::forward<F>(f));
    }

    // io :: (() → A) → IO A
    //
    // Alias for effect, providing ergonomic API.
    // Use whichever name feels more natural in context.
    template<typename F>
        requires std::invocable<F>
    auto io(F&& f) -> IO<std::invoke_result_t<F>> {
        return effect(std::forward<F>(f));
    }

    // ========================================================================
    // FUNCTOR INSTANCE FOR IO
    // ========================================================================
    // fmap :: (A → B) → IO A → IO B
    //
    // Transforms the eventual result of an IO action.
    // The transformation is PURE - it doesn't introduce new effects.
    //
    // LAZY EVALUATION: The function f is not applied until unsafeRun().
    //
    // Pattern: fmap(f, IO(comp)) = IO([=]{ return f(comp()); })
    //
    // Functor Laws:
    // 1. Identity: fmap(id, io) ≡ io
    // 2. Composition: fmap(g∘f, io) ≡ fmap(g, fmap(f, io))
    //
    // Example:
    //   IO<int> readInt = effect([]{ return 42; });
    //   IO<string> readString = fmap([](int x) { return std::to_string(x); }, readInt);
    //   readString.unsafeRun();  // "42"
    //
    // Time complexity: O(1) to construct, O(f + comp) to execute
    // Space complexity: O(1)
    template<typename Func, typename A>
        requires std::invocable<Func, A>
    auto fmap(Func&& f, IO<A>&& io) {
        using B = std::invoke_result_t<Func, A>;

        auto io_ptr = std::make_shared<IO<A>>(std::move(io));
        return IO<B>([io_ptr, f = std::forward<Func>(f)]() mutable -> B {
            return f(std::move(*io_ptr).unsafeRun());
        });
    }


    // ========================================================================
    // APPLICATIVE INSTANCE FOR IO
    // ========================================================================

    // apply :: IO (A → B) → IO A → IO B
    //
    // Applies a function in IO context to a value in IO context.
    // SEQUENCES effects: first runs io_f, then runs io_a, then applies.
    //
    // CRITICAL: Effects execute in left-to-right order (io_f THEN io_a).
    //
    // Pattern: apply(IO(f), IO(a)) = IO([=]{ return f()(a()); })
    //
    // Example:
    //   IO<std::function<int(int)>> io_f = effect([]{
    //       return [](int x) { return x * 2; };
    //   });
    //   IO<int> io_a = effect([]{ return 21; });
    //   IO<int> result = apply(io_f, io_a);
    //   result.unsafeRun();  // 42
    //
    // Time complexity: O(1) to construct, O(io_f + io_a) to execute
    // Space complexity: O(1)
    template<typename Func, typename A>
        requires std::invocable<Func, A>
    auto apply(IO<Func>&& io_f, IO<A>&& io_a) {
        using B = std::invoke_result_t<Func, A>;

        auto io_f_ptr = std::make_shared<IO<Func>>(std::move(io_f));
        auto io_a_ptr = std::make_shared<IO<A>>(std::move(io_a));
        return IO<B>([io_f_ptr, io_a_ptr]() mutable -> B {
            // Sequential execution: function first, then argument
            auto f = std::move(*io_f_ptr).unsafeRun();
            auto a = std::move(*io_a_ptr).unsafeRun();
            return f(a);
        });
    }


    // ========================================================================
    // MONAD INSTANCE FOR IO
    // ========================================================================

    // return_ :: A → IO A
    //
    // Alias for pure (monadic return).
    // Named return_ to avoid C++ keyword conflict.
    template<template<typename...> typename M, typename A>
        requires std::same_as<M<A>, IO<A>>
    IO<A> return_(A value) {
        return pure<M, A>(std::move(value));
    }

    // bind :: IO A → (A → IO B) → IO B
    //
    // Sequential composition of IO actions (the >>= operator in Haskell).
    // This is THE fundamental operation that makes IO a monad.
    //
    // SEMANTICS:
    // 1. Execute m to get value a
    // 2. Apply k to a to get a new IO<B> action
    // 3. Execute that action to get b
    // 4. Return b
    //
    // CRITICAL: All execution is SUSPENDED until unsafeRun().
    // bind creates a NEW description that, when run, will execute both actions.
    //
    // Pattern: bind(IO(ma), k) = IO([=]{ return k(ma()).unsafeRun(); })
    //
    // The Kleisli composition: (a → IO b) ∘ (a' → IO a) = (a' → IO b)
    //
    // Example:
    //   auto action = bind(
    //       effect([]{ return readLine(); }),
    //       [](string line) {
    //           return effect([line]{
    //               writeLine("You said: " + line);
    //               return Unit{};
    //           });
    //       }
    //   );
    //   action.unsafeRun();  // NOW both effects happen
    //
    // Time complexity: O(1) to construct, O(m + k) to execute
    // Space complexity: O(1)
    template<typename A, typename Func>
        requires std::invocable<Func, A>
    auto bind(IO<A>&& m, Func&& k) {
        using IOB = std::invoke_result_t<Func, A>;
        using B = typename IOB::value_type;

        auto m_ptr = std::make_shared<IO<A>>(std::move(m));
        return IO<B>([m_ptr, k = std::forward<Func>(k)]() mutable -> B {
            // Execute first action to get value
            A a = std::move(*m_ptr).unsafeRun();
            // Apply continuation to get second action
            IOB io_b = k(std::move(a));
            // Execute second action to get result
            return std::move(io_b).unsafeRun();
        });
    }


    // ========================================================================
    // IO<void> SPECIALIZATION
    // ========================================================================
    // Many IO actions don't produce meaningful values (e.g., print, write file).
    // C++ void is special and requires template specialization.

    // Template specialization for IO<void>
    template<>
    class IO<void> {
    public:
        using value_type = void;

    private:
        std::function<void()> computation_;

    public:
        // Construct from void-returning function
        explicit IO(std::function<void()> comp)
            : computation_(std::move(comp)) {}

        template<typename F>
            requires std::invocable<F> &&
                     std::same_as<std::invoke_result_t<F>, void>
        explicit IO(F&& f)
            : computation_(std::forward<F>(f)) {}

        // Move-only semantics
        IO(const IO&) = delete;
        IO(IO&&) noexcept = default;
        IO& operator=(const IO&) = delete;
        IO& operator=(IO&&) noexcept = default;

        // Execute the void-returning effect
        void unsafeRun() const {
            computation_();
        }
    };

    // Helper: Create IO<Unit> from void-returning function
    // This is kept for internal use with Unit
    //
    // Example:
    //   auto print = effect_unit([]{ std::cout << "Hello\n"; });
    //   // Type: IO<Unit>
    template<typename F>
        requires std::invocable<F> &&
                 std::same_as<std::invoke_result_t<F>, void>
    IO<Unit> effect_unit(F&& f) {
        return IO<Unit>([f = std::forward<F>(f)]() -> Unit {
            f();
            return Unit{};
        });
    }

    // Specialized bind for IO<void> followed by any IO<B>
    // Allows chaining void effects more naturally
    //
    // Example:
    //   bind(printLine("Hello"), [](auto) { return printLine("World"); })
    template<typename Func>
        requires std::invocable<Func>
    auto bind(IO<void>&& m, Func&& k) {
        using IOB = std::invoke_result_t<Func>;
        using B = typename IOB::value_type;

        auto m_ptr = std::make_shared<IO<void>>(std::move(m));
        if constexpr (std::same_as<B, void>) {
            return IO<void>([m_ptr, k = std::forward<Func>(k)]() mutable {
                std::move(*m_ptr).unsafeRun();
                std::move(k)().unsafeRun();
            });
        } else {
            return IO<B>([m_ptr, k = std::forward<Func>(k)]() mutable -> B {
                std::move(*m_ptr).unsafeRun();
                IOB io_b = k();
                return std::move(io_b).unsafeRun();
            });
        }
    }

    // Specialized bind for IO<Unit> followed by any IO<B>
    // Allows chaining unit effects more naturally
    //
    // Example:
    //   bind(printLine("Hello"), [](auto) { return printLine("World"); })
    inline auto bind(IO<Unit>&& m, std::invocable<Unit> auto&& k) {
        using IOB = std::invoke_result_t<decltype(k), Unit>;
        using B = typename IOB::value_type;

        auto m_ptr = std::make_shared<IO<Unit>>(std::move(m));
        return IO<B>([m_ptr, k = std::forward<decltype(k)>(k)]() mutable -> B {
            (void)std::move(*m_ptr).unsafeRun();  // Discard Unit result (cast to void to avoid nodiscard warning)
            IOB io_b = k(Unit{});
            return std::move(io_b).unsafeRun();
        });
    }

    // ========================================================================
    // MONADIC COMBINATORS
    // ========================================================================

    // sequence :: IO A → IO B → IO B
    //
    // Executes two IO actions in sequence, discarding the first result.
    // Haskell operator: (>>)
    //
    // Example:
    //   sequence(printLine("First"), printLine("Second"))
    template<typename A, typename B>
    IO<B> sequence(IO<A>&& first, IO<B>&& second) {
        return bind(std::move(first), [second = std::move(second)](auto) mutable {
            return std::move(second);
        });
    }

    // ========================================================================
    // CONSOLE I/O PRIMITIVES
    // ========================================================================

    // putStrLn :: String → IO ()
    //
    // Prints a string followed by a newline to standard output.
    //
    // Example:
    //   putStrLn("Hello, World!").unsafeRun();
    inline IO<void> putStrLn(const std::string& str) {
        return IO<void>([str]() {
            std::cout << str << '\n';
        });
    }

    // putStr :: String → IO ()
    //
    // Prints a string to standard output without a newline.
    //
    // Example:
    //   putStr("Hello").unsafeRun();
    inline IO<void> putStr(const std::string& str) {
        return IO<void>([str]() {
            std::cout << str;
        });
    }

    // getLine :: IO String
    //
    // Reads a line from standard input.
    //
    // Example:
    //   std::string line = getLine().unsafeRun();
    inline IO<std::string> getLine() {
        return effect([]() {
            std::string line;
            std::getline(std::cin, line);
            return line;
        });
    }

    // print :: Show a => a → IO ()
    //
    // Generic printing function (uses operator<<).
    //
    // Example:
    //   print(42).unsafeRun();
    //   print("Hello").unsafeRun();
    template<typename T>
        requires requires(std::ostream& os, const T& value) {
            { os << value } -> std::convertible_to<std::ostream&>;
        }
    IO<void> print(const T& value) {
        return IO<void>([value]() {
            std::cout << value << '\n';
        });
    }

    // ========================================================================
    // FILE I/O PRIMITIVES
    // ========================================================================

    // readFile :: FilePath → IO String
    //
    // Reads entire file contents as a string.
    //
    // Example:
    //   std::string content = readFile("data.txt").unsafeRun();
    inline IO<std::string> readFile(const std::string& filepath) {
        return effect([filepath]() {
            std::ifstream file(filepath);
            if (!file.good()) {
                throw std::runtime_error("Failed to open file: " + filepath);
            }
            std::stringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        });
    }

    // writeFile :: FilePath → String → IO ()
    //
    // Writes string to file, overwriting existing content.
    //
    // Example:
    //   writeFile("output.txt", "Hello").unsafeRun();
    inline IO<void> writeFile(const std::string& filepath, const std::string& content) {
        return IO<void>([filepath, content]() {
            std::ofstream file(filepath);
            if (!file.good()) {
                throw std::runtime_error("Failed to write file: " + filepath);
            }
            file << content;
        });
    }

    // appendFile :: FilePath → String → IO ()
    //
    // Appends string to file.
    //
    // Example:
    //   appendFile("log.txt", "New entry\n").unsafeRun();
    inline IO<void> appendFile(const std::string& filepath, const std::string& content) {
        return IO<void>([filepath, content]() {
            std::ofstream file(filepath, std::ios::app);
            if (!file.good()) {
                throw std::runtime_error("Failed to append to file: " + filepath);
            }
            file << content;
        });
    }

    // ========================================================================
    // EFFECT COMPOSITION - List Operations
    // ========================================================================

    // sequence :: [IO A] → IO [A]
    //
    // Evaluates each IO action in a list and collects the results.
    // Actions are executed left-to-right.
    //
    // Example:
    //   std::vector<IO<int>> actions = {...};
    //   std::vector<int> results = sequence(std::move(actions)).unsafeRun();
    template<typename A>
    IO<std::vector<A>> sequence(std::vector<IO<A>>&& ios) {
        auto ios_ptr = std::make_shared<std::vector<IO<A>>>(std::move(ios));
        return effect([ios_ptr]() mutable {
            std::vector<A> results;
            results.reserve(ios_ptr->size());
            for (auto& io : *ios_ptr) {
                results.push_back(std::move(io).unsafeRun());
            }
            return results;
        });
    }

    // sequence_ :: [IO A] → IO ()
    //
    // Evaluates each IO action in a list, discarding results.
    // Actions are executed left-to-right.
    //
    // Example:
    //   std::vector<IO<void>> actions = {...};
    //   sequence_(std::move(actions)).unsafeRun();
    inline IO<void> sequence_(std::vector<IO<void>>&& ios) {
        auto ios_ptr = std::make_shared<std::vector<IO<void>>>(std::move(ios));
        return IO<void>([ios_ptr]() mutable {
            for (auto& io : *ios_ptr) {
                std::move(io).unsafeRun();
            }
        });
    }

    // traverse :: (a → IO b) → [a] → IO [b]
    //
    // Maps a function that produces IO actions over a list, then sequences the results.
    //
    // Example:
    //   auto results = traverse([](int x) { return pure<IO>(x * 2); }, xs).unsafeRun();
    template<typename Func, typename A>
        requires std::invocable<Func, A>
    auto traverse(Func&& f, const std::vector<A>& xs) {
        using IOB = std::invoke_result_t<Func, A>;
        using B = typename IOB::value_type;

        return effect([f = std::forward<Func>(f), xs]() mutable {
            std::vector<B> results;
            results.reserve(xs.size());
            for (const auto& x : xs) {
                results.push_back(f(x).unsafeRun());
            }
            return results;
        });
    }

    // replicateM :: Int → IO A → IO [A]
    //
    // Repeats an IO action n times and collects the results.
    //
    // Example:
    //   auto results = replicateM(5, readInt()).unsafeRun();
    template<typename A>
    IO<std::vector<A>> replicateM(int n, IO<A>&& io) {
        auto io_ptr = std::make_shared<IO<A>>(std::move(io));
        return effect([n, io_ptr]() mutable {
            std::vector<A> results;
            results.reserve(n);
            for (int i = 0; i < n; ++i) {
                // Note: IO is move-only, so we use shared_ptr to allow multiple "calls"
                results.push_back(std::move(*io_ptr).unsafeRun());
            }
            return results;
        });
    }


    // replicateM_ :: Int → IO A → IO ()
    //
    // Repeats an IO action n times, discarding results.
    //
    // Example:
    //   replicateM_(5, printLine("Hello")).unsafeRun();
    template<typename A>
    IO<void> replicateM_(int n, IO<A>&& io) {
        auto io_ptr = std::make_shared<IO<A>>(std::move(io));
        return IO<void>([n, io_ptr]() mutable {
            for (int i = 0; i < n; ++i) {
                std::move(*io_ptr).unsafeRun();
            }
        });
    }


    // ========================================================================
    // IO COMBINATORS - Control Flow
    // ========================================================================

    // when :: Bool → IO () → IO ()
    //
    // Executes the IO action only if the condition is true.
    //
    // Example:
    //   when(x > 0, printLine("Positive")).unsafeRun();
    inline IO<void> when(bool condition, IO<void>&& io) {
        if (condition) {
            return std::move(io);
        } else {
            return IO<void>([]() {});
        }
    }


    // unless :: Bool → IO () → IO ()
    //
    // Executes the IO action only if the condition is false.
    //
    // Example:
    //   unless(x == 0, printLine("Non-zero")).unsafeRun();
    inline IO<void> unless(bool condition, IO<void>&& io) {
        return when(!condition, std::move(io));
    }


    // forever :: IO A → IO ()
    //
    // Repeats an IO action infinitely (never returns).
    //
    // WARNING: This function never terminates!
    //
    // Example:
    //   forever(printLine("Loop")).unsafeRun();  // Prints "Loop" forever
    template<typename A>
    IO<void> forever(IO<A>&& io) {
        auto io_ptr = std::make_shared<IO<A>>(std::move(io));
        return IO<void>([io_ptr]() mutable {
            while (true) {
                std::move(*io_ptr).unsafeRun();
            }
            // Never reaches here, but needed to satisfy non-void return
        });
    }


    // void_ :: IO A → IO ()
    //
    // Discards the result of an IO action, returning IO<void>.
    //
    // Example:
    //   void_(readInt()).unsafeRun();  // Reads but discards the integer
    template<typename A>
    IO<void> void_(IO<A>&& io) {
        auto io_ptr = std::make_shared<IO<A>>(std::move(io));
        return IO<void>([io_ptr]() mutable {
            (void)std::move(*io_ptr).unsafeRun();
        });
    }


    // ========================================================================
    // USAGE PATTERNS AND EXAMPLES
    // ========================================================================
    //
    // 1. BASIC IO ACTION:
    // ```cpp
    // auto greet = effect([]{
    //     std::cout << "Hello, World!\n";
    //     return Unit{};
    // });
    // // Nothing printed yet!
    // greet.unsafeRun();  // NOW it prints
    // ```
    //
    // 2. COMPOSING IO ACTIONS:
    // ```cpp
    // auto program = bind(
    //     effect([]{ return readInt(); }),
    //     [](int x) {
    //         return bind(
    //             effect([]{ return readInt(); }),
    //             [x](int y) {
    //                 return effect([=]{
    //                     std::cout << x + y << "\n";
    //                     return Unit{};
    //                 });
    //             }
    //         );
    //     }
    // );
    // program.unsafeRun();  // Read two ints and print sum
    // ```
    //
    // 3. MAPPING OVER IO:
    // ```cpp
    // IO<int> readAge = effect([]{ return std::stoi(readLine()); });
    // IO<bool> isAdult = fmap([](int age) { return age >= 18; }, readAge);
    // ```
    //
    // 4. SEPARATING DESCRIPTION FROM EXECUTION:
    // ```cpp
    // IO<Unit> logMessage(const std::string& msg) {
    //     return effect([msg]{ std::cout << msg << "\n"; return Unit{}; });
    // }
    //
    // auto program = bind(logMessage("Start"), [](auto) {
    //     return bind(doWork(), [](auto result) {
    //         return logMessage("Done");
    //     });
    // });
    //
    // // program is pure - it's just a description
    // // No logs printed yet!
    //
    // program.unsafeRun();  // NOW effects execute
    // ```
    //
    // ========================================================================

} // namespace fp20
