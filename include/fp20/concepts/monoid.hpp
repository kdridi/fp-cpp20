// ============================================================================
// FP++20: Monoid Concept Definition
// ============================================================================
// A comprehensive C++20 implementation of the Monoid abstraction from
// Abstract Algebra and Functional Programming, featuring hybrid concept
// design with pattern matching and structural requirements.
//
// ============================================================================
// ACADEMIC REFERENCES & FOUNDATIONAL LITERATURE
// ============================================================================
// Abstract Algebra:
// - Reference: [nLab: Monoid] - https://ncatlab.org/nlab/show/monoid
// - Reference: [Abstract Algebra] - Dummit & Foote (Chapter 1: Groups)
// - Reference: [Categories for the Working Mathematician] - Saunders Mac Lane
//   DOI: 10.1007/978-1-4757-4721-8
//
// Functional Programming:
// - Reference: [Haskell Monoid Documentation]
//   https://hackage.haskell.org/package/base/docs/Data-Monoid.html
// - Reference: [Typeclassopedia] - Brent Yorgey (The Monad.Reader Issue 13)
//   https://wiki.haskell.org/Typeclassopedia#Monoid
// - Reference: [Learn You a Haskell: Functors, Applicative Functors and Monoids]
//   http://learnyouahaskell.com/functors-applicative-functors-and-monoids
//
// Category Theory:
// - Reference: [Monoidal Categories] - https://ncatlab.org/nlab/show/monoidal+category
// - Reference: [Category Theory for Programmers] - Bartosz Milewski
//   https://bartoszmilewski.com/2015/07/21/monoidal-categories/
//
// C++ Design:
// - Reference: [C++20 Concepts] - ISO/IEC 14882:2020
//   https://en.cppreference.com/w/cpp/language/constraints
//
// ============================================================================
// MONOID LAWS (must be satisfied by all implementations)
// ============================================================================
// A Monoid consists of:
// - A set M (the monoid type)
// - A binary associative operation ⊕ :: M → M → M (mappend)
// - An identity element ε :: M (mempty)
//
// Laws:
// 1. Left Identity: mempty ⊕ x = x
//    - Example: "" + "hello" = "hello"
//    - Example: 0 + 5 = 5
//
// 2. Right Identity: x ⊕ mempty = x
//    - Example: "hello" + "" = "hello"
//    - Example: 5 + 0 = 5
//
// 3. Associativity: (x ⊕ y) ⊕ z = x ⊕ (y ⊕ z)
//    - Example: ("a" + "b") + "c" = "a" + ("b" + "c")
//    - Example: (1 + 2) + 3 = 1 + (2 + 3)
//
// Note: These laws cannot be enforced at compile-time in C++ but must be
// respected by implementers to maintain mathematical consistency.
//
// ============================================================================
// COMMON MONOID INSTANCES
// ============================================================================
// Standard Monoids:
// - std::string with concatenation (mempty = "", mappend = +)
// - std::vector<T> with concatenation (mempty = {}, mappend = append)
// - Sum<T> with addition (mempty = 0, mappend = +)
// - Product<T> with multiplication (mempty = 1, mappend = *)
// - All<bool> with logical AND (mempty = true, mappend = &&)
// - Any<bool> with logical OR (mempty = false, mappend = ||)
//
// ============================================================================

#pragma once

#include <concepts>
#include <type_traits>
#include <vector>
#include <string>
#include <functional>

namespace fp20 {
    namespace concepts {
        // ====================================================================
        // MONOID CONCEPT OVERVIEW
        // ====================================================================
        // A Monoid is an algebraic structure consisting of:
        // 1. A type M
        // 2. An associative binary operation (mappend): M × M → M
        // 3. An identity element (mempty): M
        //
        // Monoids appear throughout mathematics and computing:
        // - String concatenation
        // - List concatenation
        // - Numeric addition/multiplication
        // - Function composition
        // - Set union/intersection
        //
        // ====================================================================
        // DESIGN RATIONALE
        // ====================================================================
        // We use a hybrid approach:
        // 1. Pattern matching for known standard monoids (string, vector)
        // 2. Opt-in mechanism for custom monoid types
        //
        // This prevents false positives while allowing user extensibility.

        // ====================================================================
        // TEMPLATE PATTERN DETECTION
        // ====================================================================

        // Forward declarations for mempty and mappend
        template<typename M>
        M mempty();

        template<typename M>
        M mappend(M a, M b);

        // ====================================================================
        // EXTENSIBILITY MECHANISM - OPT-IN TRAIT
        // ====================================================================
        // Custom types can become monoids by:
        // 1. Specializing is_monoid_opt_in<YourType> to std::true_type
        // 2. Providing mempty<YourType>() specialization
        // 3. Providing mappend(YourType, YourType) overload
        //
        // Example:
        // ```cpp
        // template<typename T>
        // struct Sum { T value; };
        //
        // namespace fp20::concepts {
        //     template<typename T>
        //     struct is_monoid_opt_in<Sum<T>> : std::true_type {};
        // }
        //
        // namespace fp20 {
        //     template<typename T>
        //     Sum<T> mempty() { return Sum<T>{T{}}; }
        //
        //     template<typename T>
        //     Sum<T> mappend(Sum<T> a, Sum<T> b) { return Sum<T>{a.value + b.value}; }
        // }
        // ```

        template<typename T>
        struct is_monoid_opt_in : std::false_type {};

        // ====================================================================
        // PATTERN MATCHING FOR STANDARD TYPES
        // ====================================================================

        // std::string is a monoid under concatenation
        template<>
        struct is_monoid_opt_in<std::string> : std::true_type {};

        // std::vector<T> is a monoid under concatenation
        template<typename T>
        struct is_monoid_opt_in<std::vector<T>> : std::true_type {};

        // ====================================================================
        // MONOID CONCEPT DEFINITION
        // ====================================================================
        // A type M satisfies Monoid if:
        // 1. It matches a known monoid pattern (string, vector), OR
        // 2. It explicitly opts in via is_monoid_opt_in
        //
        // Note: We use opt-in rather than requires expressions to check for
        // mempty/mappend existence because that would create circular dependencies
        // (the functions themselves would require the Monoid concept).
        //
        // This follows the same hybrid pattern as Functor/Applicative/Monad:
        // - Known types are recognized via pattern matching (fast, safe)
        // - Custom types opt-in via specialization (explicit, extensible)
        // - Compile errors occur naturally if mempty/mappend are missing

        template<typename M>
        concept Monoid = is_monoid_opt_in<M>::value;

    } // namespace concepts

    // Export Monoid concept to fp20 namespace
    using concepts::Monoid;

    // ========================================================================
    // MONOID OPERATIONS - GENERIC INTERFACE
    // ========================================================================
    // These functions provide the monoid operations for all types satisfying
    // the Monoid concept. Specific implementations are provided below via
    // function overloading and template specialization.
    //
    // Note: These forward declarations do NOT use requires Monoid<M> to avoid
    // circular dependencies. The concept checks for opt-in, and if you try
    // to use mempty/mappend with a non-Monoid type, you'll get a clear
    // "no matching function" error at the call site.

    // mempty<M>() :: M
    // Returns the identity element for monoid M
    template<typename M>
    M mempty();

    // mappend :: M -> M -> M
    // Combines two monoid values associatively
    template<typename M>
    M mappend(M a, M b);

    // ========================================================================
    // STANDARD MONOID IMPLEMENTATIONS
    // ========================================================================

    // ------------------------------------------------------------------------
    // std::string Monoid (Concatenation)
    // ------------------------------------------------------------------------
    // mempty = ""
    // mappend = string concatenation

    template<>
    inline std::string mempty<std::string>() {
        return std::string{};
    }

    inline std::string mappend(std::string a, std::string b) {
        return a + b;
    }

    // ------------------------------------------------------------------------
    // std::vector<T> Monoid (Concatenation)
    // ------------------------------------------------------------------------
    // mempty = empty vector
    // mappend = vector concatenation

    // Explicit specializations for common vector types to avoid ambiguity
    template<>
    inline std::vector<int> mempty<std::vector<int>>() {
        return std::vector<int>{};
    }

    template<>
    inline std::vector<std::string> mempty<std::vector<std::string>>() {
        return std::vector<std::string>{};
    }

    template<typename T>
    inline std::vector<T> mappend(std::vector<T> a, const std::vector<T>& b) {
        a.insert(a.end(), b.begin(), b.end());
        return a;
    }

    // ========================================================================
    // MONOID WRAPPER TYPES - Multiple Monoid Instances
    // ========================================================================
    // Some types (like integers) have multiple valid monoid structures.
    // We use the newtype pattern (wrapper types) to distinguish them:
    // - Sum<T>: monoid under addition (0, +)
    // - Product<T>: monoid under multiplication (1, *)
    //
    // This follows Haskell's approach where Sum and Product are newtype
    // wrappers that provide different Monoid instances for numeric types.

    // ------------------------------------------------------------------------
    // Sum<T> - Addition Monoid
    // ------------------------------------------------------------------------
    // Provides a monoid instance for numeric types using addition.
    // Identity: 0
    // Operation: +
    //
    // Example:
    // ```cpp
    // Sum<int> s1{5};
    // Sum<int> s2{7};
    // auto s3 = mappend(s1, s2);  // Sum{12}
    // auto zero = mempty<Sum<int>>();  // Sum{0}
    // ```

    template<typename T>
    struct Sum {
        T value;

        constexpr Sum() : value(T{}) {}  // Default to 0
        constexpr explicit Sum(T v) : value(v) {}

        constexpr bool operator==(const Sum&) const = default;
    };

    // Opt-in Sum<T> as a Monoid
    namespace concepts {
        template<typename T>
        struct is_monoid_opt_in<Sum<T>> : std::true_type {};
    }

    // mempty for Sum: additive identity (0)
    template<>
    inline Sum<int> mempty<Sum<int>>() {
        return Sum<int>{};  // Constructs Sum{0}
    }

    // mappend for Sum: addition
    template<typename T>
    inline Sum<T> mappend(Sum<T> a, Sum<T> b) {
        return Sum<T>{a.value + b.value};
    }

    // ------------------------------------------------------------------------
    // Product<T> - Multiplication Monoid
    // ------------------------------------------------------------------------
    // Provides a monoid instance for numeric types using multiplication.
    // Identity: 1
    // Operation: *
    //
    // Example:
    // ```cpp
    // Product<int> p1{3};
    // Product<int> p2{4};
    // auto p3 = mappend(p1, p2);  // Product{12}
    // auto one = mempty<Product<int>>();  // Product{1}
    // ```

    template<typename T>
    struct Product {
        T value;

        constexpr Product() : value(T{1}) {}  // Default to 1
        constexpr explicit Product(T v) : value(v) {}

        constexpr bool operator==(const Product&) const = default;
    };

    // Opt-in Product<T> as a Monoid
    namespace concepts {
        template<typename T>
        struct is_monoid_opt_in<Product<T>> : std::true_type {};
    }

    // mempty for Product: multiplicative identity (1)
    template<>
    inline Product<int> mempty<Product<int>>() {
        return Product<int>{};  // Constructs Product{1}
    }

    // mappend for Product: multiplication
    template<typename T>
    inline Product<T> mappend(Product<T> a, Product<T> b) {
        return Product<T>{a.value * b.value};
    }

    // ========================================================================
    // UTILITY FUNCTIONS
    // ========================================================================

    // mconcat :: [M] -> M
    // Fold a list of monoid values using mappend, starting with mempty
    template<Monoid M>
    M mconcat(const std::vector<M>& values) {
        M result = mempty<M>();
        for (const auto& v : values) {
            result = mappend(result, v);
        }
        return result;
    }

    // Overload for initializer_list
    template<Monoid M>
    M mconcat(std::initializer_list<M> values) {
        M result = mempty<M>();
        for (const auto& v : values) {
            result = mappend(result, v);
        }
        return result;
    }

} // namespace fp20

// ============================================================================
// USAGE EXAMPLES AND PATTERNS
// ============================================================================
//
// 1. STRING CONCATENATION:
// ```cpp
// using namespace fp20;
// auto empty = mempty<std::string>();  // ""
// auto hello = mappend(std::string{"Hello"}, std::string{" World"});  // "Hello World"
// auto joined = mconcat({std::string{"a"}, std::string{"b"}, std::string{"c"}});  // "abc"
// ```
//
// 2. VECTOR CONCATENATION:
// ```cpp
// auto empty_vec = mempty<std::vector<int>>();  // {}
// auto combined = mappend(std::vector{1, 2}, std::vector{3, 4});  // {1, 2, 3, 4}
// auto flattened = mconcat({std::vector{1}, std::vector{2, 3}, std::vector{4}});  // {1, 2, 3, 4}
// ```
//
// 3. CUSTOM MONOID - SUM:
// ```cpp
// template<typename T>
// struct Sum {
//     T value;
//     constexpr Sum() : value(T{}) {}
//     constexpr explicit Sum(T v) : value(v) {}
//     constexpr bool operator==(const Sum&) const = default;
// };
//
// namespace fp20::concepts {
//     template<typename T>
//     struct is_monoid_opt_in<Sum<T>> : std::true_type {};
// }
//
// namespace fp20 {
//     template<typename T>
//     Sum<T> mempty() { return Sum<T>{}; }
//
//     template<typename T>
//     Sum<T> mappend(Sum<T> a, Sum<T> b) {
//         return Sum<T>{a.value + b.value};
//     }
// }
//
// // Usage:
// auto zero = mempty<Sum<int>>();  // Sum{0}
// auto sum = mappend(Sum{5}, Sum{7});  // Sum{12}
// auto total = mconcat({Sum{1}, Sum{2}, Sum{3}});  // Sum{6}
// ```
//
// 4. MONOID IN WRITER MONAD:
// ```cpp
// // Writer<W, A> requires W to be a Monoid
// Writer<std::string, int> w1 = writer(42, std::string{"log1"});
// Writer<std::string, int> w2 = writer(100, std::string{" log2"});
// // Logs are combined with mappend when using bind/apply
// ```
//
// ============================================================================
