// ============================================
// FP++20: Applicative Concept Definition
// ============================================
// A comprehensive C++20 implementation of the Applicative Functor abstraction,
// extending Functor with the ability to lift multi-parameter functions and
// sequence computations with effects.
//
// ============================================
// ACADEMIC REFERENCES & FOUNDATIONAL LITERATURE
// ============================================
// Seminal Papers:
// - Reference: [Applicative Programming with Effects] - McBride & Paterson (2008)
//   Journal of Functional Programming, 18(1), 1-13
//   DOI: 10.1017/S0956796807006326
//   The foundational paper introducing Applicative Functors
//
// Category Theory:
// - Reference: [nLab: Applicative Functor] - https://ncatlab.org/nlab/show/applicative+functor
// - Reference: [Monoidal Functors] - https://ncatlab.org/nlab/show/monoidal+functor
//   Applicatives are lax monoidal functors in category theory
// - Reference: [Category Theory for Programmers] - Bartosz Milewski
//   https://bartoszmilewski.com/2017/02/06/applicative-functors/
//
// Functional Programming:
// - Reference: [Haskell Applicative] - https://wiki.haskell.org/Applicative_functor
// - Reference: [Typeclassopedia] - Brent Yorgey (The Monad.Reader Issue 13)
//   https://wiki.haskell.org/Typeclassopedia#Applicative
// - Reference: [Learn You a Haskell: Applicative Functors] - Miran Lipovača
//   http://learnyouahaskell.com/functors-applicative-functors-and-monoids#applicative-functors
//
// C++ Concepts and Design:
// - Reference: [C++20 Concepts: Subsumption] - ISO/IEC 14882:2020
//   https://en.cppreference.com/w/cpp/language/constraints#Partial_ordering_of_constraints
// - Reference: [Concepts TS] - Andrew Sutton
//   http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4361.pdf
//
// ============================================
// APPLICATIVE LAWS (must be satisfied by all implementations)
// ============================================
// 1. Identity Law: pure id <*> v = v
//    - Applying the identity function wrapped in the applicative context
//      to any value returns that value unchanged
//    - Example: apply(pure<vector>(identity), v) == v
//
// 2. Homomorphism Law: pure f <*> pure x = pure (f x)
//    - Applying a pure function to a pure value is the same as
//      applying the function directly and then lifting the result
//    - Example: apply(pure(f), pure(x)) == pure(f(x))
//
// 3. Interchange Law: u <*> pure y = pure ($ y) <*> u
//    - The order of evaluation can be interchanged when one side is pure
//    - Example: apply(funcs, pure(y)) == apply(pure(apply_to_y), funcs)
//
// 4. Composition Law: pure (.) <*> u <*> v <*> w = u <*> (v <*> w)
//    - Function composition distributes over apply
//    - Enables optimization by fusing operations
//
// 5. Functor Relationship: fmap f x = pure f <*> x
//    - Every Applicative operation can express fmap
//    - This ensures Applicative properly extends Functor
//
// Note: These laws cannot be enforced at compile-time in C++ but must be
// respected by implementers to maintain mathematical consistency.

#pragma once

#include <fp20/concepts/functor.hpp>
#include <concepts>
#include <type_traits>
#include <vector>
#include <optional>
#include <functional>

namespace fp20 {
    namespace concepts {
        // ============================================
        // APPLICATIVE FUNCTOR CONCEPT OVERVIEW
        // ============================================
        // An Applicative Functor is a computational context that can:
        // 1. Wrap a pure value (pure :: a -> F a)
        // 2. Apply a wrapped function to a wrapped value (apply :: F (a -> b) -> F a -> F b)
        //
        // KEY INSIGHT: While Functor lets us map single-parameter functions,
        // Applicative enables multi-parameter function lifting through currying
        // and sequential application.
        //
        // Mathematical Foundation:
        // In category theory, an applicative functor is a lax monoidal functor
        // between the cartesian monoidal structure on types and a monoidal
        // structure on the functor category.
        //
        // ============================================
        // CONCEPT SUBSUMPTION IN C++20
        // ============================================
        // C++20 concepts support subsumption - a partial ordering of constraints
        // where more constrained concepts subsume less constrained ones.
        //
        // HOW SUBSUMPTION WORKS:
        // -----------------------
        // When concept A requires concept B (A subsumes B):
        // - Any type satisfying A also satisfies B
        // - In overload resolution, A is more specialized than B
        // - The compiler prefers the more constrained overload
        //
        // APPLICATIVE SUBSUMES FUNCTOR:
        // ------------------------------
        // template<typename F>
        // concept Applicative = Functor<F> && ...
        //
        // This means:
        // 1. Every Applicative is automatically a Functor
        // 2. Generic algorithms requiring Functor work with Applicatives
        // 3. Specialized Applicative algorithms override Functor versions
        //
        // BENEFITS OF SUBSUMPTION:
        // ------------------------
        // - Type Safety: Can't have Applicative without Functor operations
        // - Code Reuse: Functor algorithms work on Applicatives
        // - Extensibility: Add more specific concepts (e.g., Monad) later
        // - Performance: Compiler selects optimal overload automatically
        //
        // Example demonstrating subsumption:
        // ```cpp
        // template<Functor F>
        // void process(F f) { /* generic functor algorithm */ }
        //
        // template<Applicative A>
        // void process(A a) { /* optimized for applicatives */ }
        //
        // std::vector<int> v;
        // process(v);  // Selects Applicative version (more specialized)
        // ```
        //
        // ============================================
        // HYBRID DESIGN APPROACH (inherited from Functor)
        // ============================================
        // Like Functor, Applicative uses a hybrid approach:
        // 1. Pattern matching for standard library types
        // 2. Structural requirements for custom types
        //
        // This ensures:
        // - Fast compile-time recognition of std::vector, std::optional
        // - Extensibility for user-defined applicative types
        // - Prevention of false positives from coincidental structure
        //
        // See functor.hpp for detailed rationale on the hybrid approach.

        // ============================================
        // TEMPLATE PATTERN DETECTION
        // ============================================
        // Pattern matching for known applicative types.
        // Note: We use separate traits from Functor to allow independent
        // control over which types support which operations.

        template<typename T>
        struct is_template_instance_of_vector_applicative : std::false_type {};

        template<typename T>
        struct is_template_instance_of_vector_applicative<std::vector<T>> : std::true_type {};

        template<typename T>
        struct is_template_instance_of_optional_applicative : std::false_type {};

        template<typename T>
        struct is_template_instance_of_optional_applicative<std::optional<T>> : std::true_type {};

        // ============================================
        // EXTENSIBILITY MECHANISM FOR APPLICATIVES
        // ============================================
        // Custom types can opt-in to being Applicative functors.
        //
        // COMPLETE EXTENSIBILITY EXAMPLE - Custom Result Type:
        // ```cpp
        // template<typename T, typename E>
        // class Result {
        // public:
        //     using value_type = T;
        //     using error_type = E;
        //
        //     Result() : is_ok_(false) {}  // Default constructible
        //     explicit Result(T value) : is_ok_(true), value_(std::move(value)) {}
        //     explicit Result(E error) : is_ok_(false), error_(std::move(error)) {}
        //
        //     bool is_ok() const { return is_ok_; }
        //     const T& value() const { return value_; }
        //     const E& error() const { return error_; }
        //
        // private:
        //     bool is_ok_;
        //     union {
        //         T value_;
        //         E error_;
        //     };
        // };
        //
        // // Step 1: Opt-in as Functor (prerequisite)
        // namespace fp20::concepts {
        //     template<typename T, typename E>
        //     struct is_functor_opt_in<Result<T, E>> : std::true_type {};
        //
        //     template<typename T, typename E>
        //     struct is_applicative_opt_in<Result<T, E>> : std::true_type {};
        // }
        //
        // // Step 2: Implement fmap (Functor requirement)
        // namespace fp20 {
        //     template<typename Func, typename T, typename E>
        //     auto fmap(Func&& f, const Result<T, E>& result) {
        //         using ResultType = decltype(f(std::declval<T>()));
        //         if (result.is_ok()) {
        //             return Result<ResultType, E>{f(result.value())};
        //         }
        //         return Result<ResultType, E>{result.error()};
        //     }
        //
        //     // Step 3: Implement pure
        //     template<template<typename...> typename F, typename T, typename E = std::string>
        //         requires std::same_as<F<T, E>, Result<T, E>>
        //     auto pure(T&& value) -> Result<std::decay_t<T>, E> {
        //         return Result<std::decay_t<T>, E>{std::forward<T>(value)};
        //     }
        //
        //     // Step 4: Implement apply
        //     template<typename Func, typename T, typename E>
        //     auto apply(const Result<Func, E>& func_result,
        //                const Result<T, E>& value_result) {
        //         using ResultType = decltype(std::declval<Func>()(std::declval<T>()));
        //         if (func_result.is_ok() && value_result.is_ok()) {
        //             return Result<ResultType, E>{func_result.value()(value_result.value())};
        //         }
        //         // Propagate first error
        //         if (!func_result.is_ok()) {
        //             return Result<ResultType, E>{func_result.error()};
        //         }
        //         return Result<ResultType, E>{value_result.error()};
        //     }
        // }
        // ```
        //
        // BEST PRACTICES FOR CUSTOM APPLICATIVES:
        // - Ensure Functor laws hold first
        // - Implement pure to create minimal context
        // - Apply should sequence effects appropriately
        // - Consider error propagation semantics carefully
        // - Document the specific semantics of your applicative

        template<typename T>
        struct is_applicative_opt_in : std::false_type {};

        // Helper: detect if a type is a known applicative via pattern matching
        template<typename T>
        struct is_known_applicative : std::bool_constant<
            is_template_instance_of_vector_applicative<T>::value ||
            is_template_instance_of_optional_applicative<T>::value ||
            is_applicative_opt_in<T>::value
        > {};

        // ============================================
        // STRUCTURAL REQUIREMENTS FOR APPLICATIVES
        // ============================================
        // An Applicative must first be a Functor, then add additional capabilities.
        // This structural check ensures custom types have the necessary interface.
        //
        // The Functor<F> requirement here is crucial for subsumption to work correctly.
        template<typename F>
        concept HasApplicativeStructure = requires {
            typename F::value_type;
            requires !std::same_as<F, typename F::value_type>; // Prevent recursion
        } && std::default_initializable<F> && Functor<F>;

        // ============================================
        // PRIMARY APPLICATIVE CONCEPT
        // ============================================
        // The main concept identifying Applicative Functors.
        //
        // CRITICAL: The Functor<F> constraint enables subsumption.
        // This means Applicative is MORE SPECIALIZED than Functor,
        // allowing the compiler to prefer Applicative-specific algorithms
        // when both are available.
        //
        // Type resolution example:
        // - std::vector<int> satisfies Functor<F> ✓
        // - std::vector<int> pattern matches as applicative ✓
        // - Therefore: std::vector<int> is an Applicative ✓
        template<typename F>
        concept Applicative =
            Functor<F> &&  // SUBSUMPTION: Every Applicative is a Functor
            (is_known_applicative<F>::value || HasApplicativeStructure<F>);
    }

    // ============================================
    // PURE FUNCTION - Lifting Values into Applicative Context
    // ============================================
    // pure :: a -> F a
    //
    // Wraps a plain value into an Applicative context with minimal effects.
    // This is the unit of the monoidal structure in category theory terms.
    //
    // SEMANTIC MEANING BY TYPE:
    // - Lists: Singleton list containing just the value
    // - Maybe/Optional: Some/Just wrapping the value
    // - Either/Result: Success wrapping the value
    // - Parser: Parser that consumes no input and returns the value
    // - IO: IO action that performs no side effects and returns the value
    //
    // Mathematical Foundation:
    // Pure is the natural transformation η: Id → F where Id is the identity functor.
    // Reference: [Natural Transformations] - https://ncatlab.org/nlab/show/natural+transformation
    //
    // DESIGN NOTE: We use template template parameters to infer the
    // applicative type, allowing syntax like: pure<std::vector>(42)

    // ============================================
    // PURE FOR STD::VECTOR
    // ============================================
    // Creates a singleton vector - the minimal non-empty vector context.
    // This represents "pure" computation with no alternative outcomes.
    //
    // Example usage:
    // ```cpp
    // auto singleton = pure<std::vector>(42);  // vector{42}
    // auto funcs = pure<std::vector>([](int x) { return x * 2; });
    // ```
    template<template<typename...> typename F, typename T>
        requires std::same_as<F<T>, std::vector<T>>
    auto pure(T&& value) -> std::vector<std::decay_t<T>> {
        return std::vector<std::decay_t<T>>{std::forward<T>(value)};
    }

    // ============================================
    // PURE FOR STD::OPTIONAL
    // ============================================
    // Wraps a value in Some/Just - representing successful computation.
    // This is the identity element for the applicative operation.
    //
    // Example usage:
    // ```cpp
    // auto maybe_int = pure<std::optional>(42);  // optional{42}
    // auto maybe_func = pure<std::optional>([](int x) { return x * 2; });
    // ```
    template<template<typename...> typename F, typename T>
        requires std::same_as<F<T>, std::optional<T>>
    constexpr auto pure(T&& value) -> std::optional<std::decay_t<T>> {
        return std::optional<std::decay_t<T>>{std::forward<T>(value)};
    }

    // ============================================
    // APPLY FUNCTION - Sequential Application with Effects
    // ============================================
    // apply :: F (a -> b) -> F a -> F b
    // Also written as (<*>) in Haskell
    //
    // Applies wrapped function(s) to wrapped value(s), sequencing any effects.
    // This is the tensor product ⊗ of the monoidal structure.
    //
    // KEY INSIGHT: Apply enables lifting of multi-parameter functions:
    // ```cpp
    // // Lifting a 3-parameter function:
    // auto add3 = [](int a) { return [=](int b) { return [=](int c) {
    //     return a + b + c;
    // }; }; };
    //
    // auto result = apply(apply(apply(pure<std::optional>(add3),
    //                                  optional{1}),
    //                           optional{2}),
    //                    optional{3});  // optional{6}
    // ```
    //
    // Mathematical Foundation:
    // Apply implements the monoidal product in the functor category.
    // Reference: [Monoidal Categories] - https://ncatlab.org/nlab/show/monoidal+category

    // ============================================
    // APPLY FOR STD::OPTIONAL
    // ============================================
    // Propagates None through the computation chain.
    // Only applies the function if both operands are Some.
    //
    // Semantics:
    // - Some(f) <*> Some(x) = Some(f(x))
    // - None <*> _ = None
    // - _ <*> None = None
    //
    // This implements short-circuiting evaluation for optional values.
    //
    // Example - Safe division:
    // ```cpp
    // auto safe_div = [](double a) {
    //     return [=](double b) -> std::optional<double> {
    //         return b != 0 ? std::optional(a / b) : std::nullopt;
    //     };
    // };
    // auto result = apply(fmap(safe_div, optional{10.0}), optional{2.0});
    // ```
    template<typename Func, typename T>
    constexpr auto apply(
        const std::optional<Func>& opt_func,
        const std::optional<T>& opt_value
    ) -> std::optional<decltype(std::declval<Func>()(std::declval<T>()))> {
        using ResultType = decltype(std::declval<Func>()(std::declval<T>()));

        if (opt_func.has_value() && opt_value.has_value()) {
            return std::optional<ResultType>{(*opt_func)(*opt_value)};
        }
        return std::nullopt;
    }

    // ============================================
    // APPLY FOR STD::VECTOR - Cartesian Product
    // ============================================
    // Computes the Cartesian product of functions and values.
    // Each function is applied to each value, with results concatenated.
    //
    // Semantics:
    // [f, g] <*> [x, y] = [f(x), f(y), g(x), g(y)]
    //
    // This models non-deterministic computation where each function
    // represents a possible computation path and each value a possible input.
    //
    // Time Complexity: O(m * n) where m = funcs.size(), n = values.size()
    // Space Complexity: O(m * n) for the result
    //
    // Example - Multiple transformations:
    // ```cpp
    // std::vector<std::function<int(int)>> ops = {
    //     [](int x) { return x + 1; },
    //     [](int x) { return x * 2; }
    // };
    // std::vector<int> values = {1, 2, 3};
    // auto results = apply(ops, values);  // {2, 3, 4, 2, 4, 6}
    // ```
    //
    // Mathematical Note: This implements the Day convolution for the list monad.
    // Reference: [Day Convolution] - https://ncatlab.org/nlab/show/Day+convolution
    template<typename Func, typename T>
    auto apply(
        const std::vector<Func>& funcs,
        const std::vector<T>& values
    ) -> std::vector<decltype(std::declval<Func>()(std::declval<T>()))> {
        using ResultType = decltype(std::declval<Func>()(std::declval<T>()));
        std::vector<ResultType> result;

        // Pre-allocate for efficiency
        result.reserve(funcs.size() * values.size());

        // Cartesian product: apply each function to each value
        // Order matters for law compliance: functions outer, values inner
        for (const auto& func : funcs) {
            for (const auto& value : values) {
                result.push_back(func(value));
            }
        }

        return result;
    }

    // ============================================
    // ADVANCED USAGE PATTERNS
    // ============================================
    // Demonstration of Applicative patterns in functional programming:
    //
    // 1. LIFTING BINARY FUNCTIONS (liftA2 pattern):
    // ```cpp
    // template<Applicative A, typename F, typename T, typename U>
    // auto liftA2(F f, const A<T>& a, const A<U>& b) {
    //     auto curried = [f](T t) { return [=](U u) { return f(t, u); }; };
    //     return apply(fmap(curried, a), b);
    // }
    //
    // // Usage:
    // auto sum = [](int a, int b) { return a + b; };
    // auto result = liftA2(sum, optional{3}, optional{4});  // optional{7}
    // ```
    //
    // 2. SEQUENCING COMPUTATIONS:
    // ```cpp
    // // Execute multiple optional computations, collecting results
    // std::vector<std::optional<int>> maybes = {
    //     optional{1}, optional{2}, std::nullopt, optional{4}
    // };
    // // Use traverse pattern to sequence optionals into optional vector
    // ```
    //
    // 3. VALIDATION WITH MULTIPLE ERRORS:
    // ```cpp
    // // Validation applicative accumulates all errors
    // auto validate_age = [](int age) -> Validation<int, string> {
    //     return age >= 0 ? Success(age) : Failure("Age must be non-negative");
    // };
    // auto validate_name = [](string name) -> Validation<string, string> {
    //     return !name.empty() ? Success(name) : Failure("Name cannot be empty");
    // };
    // // Combine validations using applicative to collect all errors
    // ```
    //
    // 4. PARSER COMBINATORS:
    // ```cpp
    // // Applicative parsers compose sequentially
    // Parser<int> parseInt = ...;
    // Parser<char> parseComma = ...;
    // auto parsePair = liftA2(make_pair, parseInt,
    //                         apply(parseComma.ignoreLeft(), parseInt));
    // ```

    // ============================================
    // RELATIONSHIP TO MONADS
    // ============================================
    // Note: Every Monad is an Applicative (and thus a Functor).
    // The hierarchy is: Functor ⊂ Applicative ⊂ Monad
    //
    // Applicatives are more general (less powerful) than Monads:
    // - Applicative: Effects are determined statically
    // - Monad: Effects can depend on runtime values (via bind/flatMap)
    //
    // Many computations only need Applicative, not full Monad power,
    // leading to more composable and analyzable code.
    //
    // Reference: [Applicative vs Monad] -
    // https://wiki.haskell.org/Typeclassopedia#Intuition_3
}