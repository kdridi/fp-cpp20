// ============================================
// FP++20: Functor Concept Definition
// ============================================
// A comprehensive C++20 implementation of the Functor abstraction from
// Category Theory and Functional Programming, featuring hybrid concept
// design with pattern matching and structural requirements.
//
// ============================================
// ACADEMIC REFERENCES & FOUNDATIONAL LITERATURE
// ============================================
// Category Theory:
// - Reference: [nLab: Functor] - https://ncatlab.org/nlab/show/functor
// - Reference: [Category Theory for Programmers] - Bartosz Milewski
//   https://bartoszmilewski.com/2015/01/20/functors/
// - Reference: [Categories for the Working Mathematician] - Saunders Mac Lane
//   DOI: 10.1007/978-1-4757-4721-8
//
// Functional Programming:
// - Reference: [Haskell Functor Documentation] - https://wiki.haskell.org/Functor
// - Reference: [Typeclassopedia] - Brent Yorgey (The Monad.Reader Issue 13)
//   https://wiki.haskell.org/Typeclassopedia#Functor
// - Reference: [Learn You a Haskell: Functors] - Miran Lipovača
//   http://learnyouahaskell.com/functors-applicative-functors-and-monoids
//
// C++ Concepts and Design:
// - Reference: [C++20 Concepts] - ISO/IEC 14882:2020
//   https://en.cppreference.com/w/cpp/language/constraints
// - Reference: [Concepts: Linguistic Support for Generic Programming] - Stroustrup, Dos Reis
//   http://www.stroustrup.com/oopsla06.pdf
//
// ============================================
// FUNCTOR LAWS (must be satisfied by all implementations)
// ============================================
// 1. Identity Law: fmap id = id
//    - Mapping the identity function over a functor returns the functor unchanged
//    - Example: fmap(identity, vector{1,2,3}) == vector{1,2,3}
//
// 2. Composition Law: fmap (f . g) = fmap f . fmap g
//    - Mapping a composition of functions equals composing the mapped functions
//    - Example: fmap(f∘g, xs) == fmap(f, fmap(g, xs))
//
// Note: These laws cannot be enforced at compile-time in C++ but must be
// respected by implementers to maintain mathematical consistency.

#pragma once

#include <concepts>
#include <type_traits>
#include <vector>
#include <optional>
#include <list>
#include <string>
#include <string_view>

namespace fp20 {
    namespace concepts {
        // ============================================
        // FUNCTOR CONCEPT OVERVIEW
        // ============================================
        // A Functor is a type constructor F that can map a function (A -> B)
        // to a function (F<A> -> F<B>), preserving structure.
        //
        // In Category Theory, a functor is a structure-preserving mapping between
        // categories. In programming, we focus on endofunctors in the category of
        // types, where objects are types and morphisms are functions.
        //
        // ============================================
        // HYBRID DESIGN APPROACH RATIONALE
        // ============================================
        // This implementation uses a hybrid approach combining:
        // 1. Template Pattern Matching (for known standard types)
        // 2. Structural Requirements (for custom user types)
        //
        // WHY HYBRID APPROACH?
        // --------------------
        // Pure Structural Checking Problems:
        // - Would accept types that shouldn't be functors (e.g., std::string has value_type)
        // - Cannot distinguish between container-like and scalar-like types reliably
        // - May accidentally accept types with coincidental structural matches
        //
        // Pure Pattern Matching Problems:
        // - Would require explicit listing of every functor type
        // - No extensibility for user-defined types
        // - Maintenance burden for library evolution
        //
        // Hybrid Approach Benefits:
        // - Known standard types are recognized via pattern matching (safe and fast)
        // - Custom types can opt-in via structural requirements
        // - Prevents false positives from scalar-like containers
        // - Compile-time efficient (no unnecessary instantiation)
        //
        // ILL-FORMED TYPE HANDLING:
        // -------------------------
        // Consider: std::vector<const int>
        // - Pattern matches as vector (template shape is correct)
        // - But instantiation would fail (vector requires non-const value_type)
        // - Our approach: Pattern match succeeds, actual usage fails with clear error
        // - Alternative would require complex SFINAE/requires expressions that slow compilation

        // ============================================
        // TEMPLATE PATTERN DETECTION
        // ============================================
        // These traits detect functor types by examining their template structure
        // WITHOUT instantiating them. This is crucial for compile-time performance
        // and handling potentially ill-formed instantiations.
        //
        // Technical Note: We use partial specialization pattern matching rather
        // than requires expressions to avoid instantiation side effects.

        template<typename T>
        struct is_template_instance_of_vector : std::false_type {};

        template<typename T>
        struct is_template_instance_of_vector<std::vector<T>> : std::true_type {};

        template<typename T>
        struct is_template_instance_of_optional : std::false_type {};

        template<typename T>
        struct is_template_instance_of_optional<std::optional<T>> : std::true_type {};

        template<typename T>
        struct is_template_instance_of_list : std::false_type {};

        template<typename T>
        struct is_template_instance_of_list<std::list<T>> : std::true_type {};

        // ============================================
        // EXTENSIBILITY MECHANISM
        // ============================================
        // Custom types can become functors by specializing this trait.
        // This provides an opt-in mechanism for user-defined types.
        //
        // EXTENSIBILITY GUIDE FOR CUSTOM TYPES:
        // -------------------------------------
        // Step 1: Define your container type with appropriate structure
        // Step 2: Specialize is_functor_opt_in for your type
        // Step 3: Implement fmap for your type
        // Step 4: Ensure functor laws are satisfied
        //
        // COMPLETE EXAMPLE - Custom Maybe Type:
        // ```cpp
        // template<typename T>
        // class Maybe {
        // public:
        //     using value_type = T;  // Required for structural check
        //
        //     Maybe() : has_value_(false) {}  // Default constructible
        //     explicit Maybe(T value) : has_value_(true), value_(std::move(value)) {}
        //
        //     bool has_value() const { return has_value_; }
        //     const T& value() const { return value_; }
        //
        // private:
        //     bool has_value_;
        //     T value_;
        // };
        //
        // // Step 2: Opt-in to Functor concept
        // namespace fp20::concepts {
        //     template<typename T>
        //     struct is_functor_opt_in<Maybe<T>> : std::true_type {};
        // }
        //
        // // Step 3: Implement fmap
        // namespace fp20 {
        //     template<typename Func, typename T>
        //     auto fmap(Func&& f, const Maybe<T>& maybe) {
        //         using ResultType = decltype(f(std::declval<T>()));
        //         if (maybe.has_value()) {
        //             return Maybe<ResultType>{f(maybe.value())};
        //         }
        //         return Maybe<ResultType>{};
        //     }
        // }
        // ```
        //
        // BEST PRACTICES:
        // - Ensure value_type accurately reflects contained type
        // - Implement fmap to preserve your container's semantics
        // - Test that functor laws hold for your implementation
        // - Consider implementing rebind_functor for better generic programming

        template<typename T>
        struct is_functor_opt_in : std::false_type {};

        // Helper: detect if a type is a known functor via pattern matching
        // This avoids instantiation and compile-time overhead
        template<typename T>
        struct is_known_functor : std::bool_constant<
            is_template_instance_of_vector<T>::value ||
            is_template_instance_of_optional<T>::value ||
            is_template_instance_of_list<T>::value ||
            is_functor_opt_in<T>::value
        > {};

        // ============================================
        // SCALAR-LIKE CONTAINER EXCLUSION
        // ============================================
        // Some standard types have container-like interfaces but shouldn't
        // be treated as functors because they represent scalar values.
        // Reference: [Design rationale] - strings are sequences of characters
        // but conceptually represent atomic text values, not containers.
        template<typename T>
        concept ScalarLikeContainer =
            std::same_as<std::remove_cv_t<T>, std::string> ||
            std::same_as<std::remove_cv_t<T>, std::string_view> ||
            std::same_as<std::remove_cv_t<T>, std::wstring> ||
            std::same_as<std::remove_cv_t<T>, std::wstring_view>;

        // ============================================
        // STRUCTURAL REQUIREMENTS FOR FUNCTORS
        // ============================================
        // For types not recognized by pattern matching, we check structural
        // requirements. This allows custom types to participate as functors.
        //
        // Requirements explained:
        // - value_type: The type of elements contained (standard container convention)
        // - Non-recursive: Prevents infinite recursion in type checking
        // - Default constructible: Enables creation of empty functors (useful for algorithms)
        template<typename F>
        concept HasFunctorStructure = requires {
            typename F::value_type;
            requires !std::same_as<F, typename F::value_type>; // Prevent recursion
        } && std::default_initializable<F>;

        // ============================================
        // PRIMARY FUNCTOR CONCEPT
        // ============================================
        // The main concept that identifies types as functors.
        // Uses hybrid approach: fast pattern matching for known types,
        // structural checking for extensibility.
        //
        // CONCEPT DESIGN NOTES:
        // - ScalarLikeContainer exclusion prevents false positives
        // - is_known_functor provides O(1) compile-time recognition
        // - HasFunctorStructure enables extensibility
        // - OR relationship allows either recognition method
        //
        // This concept will be subsumed by Applicative (see applicative.hpp)
        template<typename F>
        concept Functor =
            !ScalarLikeContainer<F> &&
            (is_known_functor<F>::value || HasFunctorStructure<F>);
    }

    // ============================================
    // TYPE TRAITS FOR FUNCTOR MANIPULATION
    // ============================================
    // These metafunctions provide type-level operations on functors,
    // essential for generic programming with functor abstractions.

    // Extract the value_type from a Functor
    // Example: functor_value_type<std::vector<int>>::type == int
    // This is the type-level equivalent of "unwrapping" a functor
    template<typename F>
    struct functor_value_type {
        using type = typename F::value_type;
    };

    // ============================================
    // FUNCTOR REBINDING
    // ============================================
    // Rebind a Functor with a new value type - essential for fmap implementation.
    // This is the type-level operation corresponding to the functor mapping.
    //
    // Mathematical correspondence:
    // If F is a functor and we have f: A -> B,
    // then rebind_functor<F<A>, B>::type gives us F<B>
    //
    // Example: rebind_functor<std::vector<int>, std::string>::type == std::vector<std::string>
    template<typename F, typename NewType>
    struct rebind_functor;

    template<typename T, typename NewType>
    struct rebind_functor<std::vector<T>, NewType> {
        using type = std::vector<NewType>;
    };

    template<typename T, typename NewType>
    struct rebind_functor<std::optional<T>, NewType> {
        using type = std::optional<NewType>;
    };

    template<typename T, typename NewType>
    struct rebind_functor<std::list<T>, NewType> {
        using type = std::list<NewType>;
    };

    // Note: Custom functors should specialize rebind_functor for full compatibility

    // ============================================
    // FMAP FUNCTION - The Fundamental Functor Operation
    // ============================================
    // fmap :: (a -> b) -> F a -> F b
    //
    // Maps a function over a functor, transforming the contained value(s)
    // while preserving the functor's structure.
    //
    // MATHEMATICAL FOUNDATION:
    // In category theory, fmap is the morphism mapping of the functor.
    // It must preserve composition and identity (see functor laws above).
    //
    // References:
    // - Reference: [Haskell fmap] - https://hackage.haskell.org/package/base/docs/Data-Functor.html
    // - Reference: [Category Theory: Functor Morphism] - https://ncatlab.org/nlab/show/functor
    //
    // IMPLEMENTATION NOTES:
    // - Each specialization must maintain the functor laws
    // - Structure preservation means: size, shape, and traversal order unchanged
    // - Only the contained values are transformed

    // ============================================
    // FMAP FOR STD::VECTOR
    // ============================================
    // Vector fmap applies the function to each element, preserving order and size.
    // Time complexity: O(n) where n = vec.size()
    // Space complexity: O(n) for the result vector
    //
    // Example usage:
    // ```cpp
    // auto square = [](int x) { return x * x; };
    // std::vector<int> nums = {1, 2, 3, 4};
    // auto squares = fmap(square, nums);  // {1, 4, 9, 16}
    // ```
    template<typename Func, typename T>
    auto fmap(Func&& f, const std::vector<T>& vec) -> std::vector<decltype(f(std::declval<T>()))> {
        using ResultType = decltype(f(std::declval<T>()));
        std::vector<ResultType> result;
        result.reserve(vec.size());  // Optimization: pre-allocate memory
        for (const auto& elem : vec) {
            result.push_back(f(elem));
        }
        return result;
    }

    // ============================================
    // FMAP FOR STD::OPTIONAL
    // ============================================
    // Optional fmap applies the function only if a value is present.
    // This implements the Maybe functor from Haskell.
    // Time complexity: O(1)
    // Space complexity: O(1)
    //
    // Semantic behavior:
    // - Some(x) -> Some(f(x))
    // - None -> None
    //
    // Example usage:
    // ```cpp
    // auto safe_divide = [](int x) -> std::optional<double> {
    //     return x != 0 ? std::optional(10.0 / x) : std::nullopt;
    // };
    // std::optional<int> maybe_num = 5;
    // auto result = fmap([](int x) { return x * 2; }, maybe_num);  // Optional(10)
    // ```
    template<typename Func, typename T>
    constexpr auto fmap(Func&& f, const std::optional<T>& opt) -> std::optional<decltype(f(std::declval<T>()))> {
        using ResultType = decltype(f(std::declval<T>()));
        if (opt.has_value()) {
            return std::optional<ResultType>{f(*opt)};
        }
        return std::nullopt;
    }

    // ============================================
    // FMAP FOR STD::LIST
    // ============================================
    // List fmap applies the function to each element, preserving order.
    // Similar to vector but with different performance characteristics.
    // Time complexity: O(n) where n = lst.size()
    // Space complexity: O(n) for the result list
    //
    // Note: Lists don't support random access, so we use sequential iteration.
    //
    // Example usage:
    // ```cpp
    // std::list<std::string> words = {"hello", "world"};
    // auto lengths = fmap([](const auto& s) { return s.length(); }, words);  // {5, 5}
    // ```
    template<typename Func, typename T>
    auto fmap(Func&& f, const std::list<T>& lst) -> std::list<decltype(f(std::declval<T>()))> {
        using ResultType = decltype(f(std::declval<T>()));
        std::list<ResultType> result;
        for (const auto& elem : lst) {
            result.push_back(f(elem));  // Note: push_back for lists is O(1)
        }
        return result;
    }

    // ============================================
    // USAGE EXAMPLES AND PATTERNS
    // ============================================
    // Common functor usage patterns in functional programming:
    //
    // 1. FUNCTION COMPOSITION WITH FUNCTORS:
    // ```cpp
    // auto add1 = [](int x) { return x + 1; };
    // auto times2 = [](int x) { return x * 2; };
    // auto composed = [=](int x) { return times2(add1(x)); };
    //
    // std::vector<int> nums = {1, 2, 3};
    // auto result1 = fmap(composed, nums);  // Single pass
    // auto result2 = fmap(times2, fmap(add1, nums));  // Two passes
    // // Both produce {4, 6, 8}, demonstrating the composition law
    // ```
    //
    // 2. OPTIONAL CHAINING:
    // ```cpp
    // std::optional<int> maybe_value = get_value();
    // auto result = fmap([](int x) { return x * x; },
    //               fmap([](int x) { return x + 1; }, maybe_value));
    // // Chains operations safely, propagating None if needed
    // ```
    //
    // 3. HETEROGENEOUS TRANSFORMATIONS:
    // ```cpp
    // std::vector<int> ids = {1, 2, 3};
    // auto names = fmap([](int id) { return fetch_name(id); }, ids);
    // // Transform vector<int> to vector<string>
    // ```
}