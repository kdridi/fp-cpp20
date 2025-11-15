// ============================================
// FP++20: Monad Concept Definition
// ============================================
// A comprehensive C++20 implementation of the Monad abstraction,
// extending Applicative with monadic bind for sequential computation
// with data-dependent effects.
//
// ============================================
// ACADEMIC REFERENCES & FOUNDATIONAL LITERATURE
// ============================================
// Category Theory:
// - Reference: [nLab: Monad] - https://ncatlab.org/nlab/show/monad
// - Reference: [Category Theory for Programmers] - Bartosz Milewski
//   https://bartoszmilewski.com/2016/12/27/monads-categorically/
// - Reference: [Categories for the Working Mathematician] - Saunders Mac Lane
//   DOI: 10.1007/978-1-4757-4721-8
//
// Functional Programming:
// - Reference: [Haskell Monad] - https://wiki.haskell.org/Monad
// - Reference: [Typeclassopedia] - Brent Yorgey (The Monad.Reader Issue 13)
//   https://wiki.haskell.org/Typeclassopedia#Monad
// - Reference: [Monads for Functional Programming] - Philip Wadler (1995)
//   https://homepages.inf.ed.ac.uk/wadler/papers/marktoberdorf/baastad.pdf
//
// C++ Concepts and Design:
// - Reference: [C++20 Concepts: Subsumption] - ISO/IEC 14882:2020
//   https://en.cppreference.com/w/cpp/language/constraints#Partial_ordering_of_constraints
//
// ============================================
// MONAD LAWS (must be satisfied by all implementations)
// ============================================
// 1. Left Identity: return a >>= f ≡ f a
//    - Lifting a value and then binding is the same as applying directly
//    - Example: bind(return_<optional>(x), f) == f(x)
//
// 2. Right Identity: m >>= return ≡ m
//    - Binding return to a monadic value returns the same value
//    - Example: bind(m, return_<optional>) == m
//
// 3. Associativity: (m >>= f) >>= g ≡ m >>= (\x -> f x >>= g)
//    - Order of binding operations doesn't matter (when nested properly)
//    - Example: bind(bind(m, f), g) == bind(m, [](auto x){ return bind(f(x), g); })
//
// Note: These laws cannot be enforced at compile-time in C++ but must be
// respected by implementers to maintain mathematical consistency.

#pragma once

#include <fp20/concepts/applicative.hpp>
#include <fp20/concepts/functor.hpp>
#include <concepts>
#include <type_traits>
#include <vector>
#include <optional>
#include <algorithm>

namespace fp20 {
    namespace concepts {
        // ============================================
        // MONAD CONCEPT OVERVIEW
        // ============================================
        // A Monad is a computational context that extends Applicative with:
        // 1. return (pure): Lift a value into monadic context (a → M a)
        // 2. bind (>>=): Sequential composition with data-dependent effects (M a → (a → M b) → M b)
        //
        // KEY INSIGHT: While Applicative allows static effect determination,
        // Monad enables effects that depend on runtime values through bind.
        //
        // Mathematical Foundation:
        // A monad is a monoid in the category of endofunctors, with:
        // - Unit: return_ (η in category theory)
        // - Multiplication: join/bind (μ in category theory)
        //
        // ============================================
        // HYBRID DESIGN APPROACH (inherited from Applicative/Functor)
        // ============================================
        // Like Functor and Applicative, Monad uses a hybrid approach:
        // 1. Pattern matching for standard library types
        // 2. Structural requirements for custom types
        //
        // This ensures:
        // - Fast compile-time recognition of std::vector, std::optional
        // - Extensibility for user-defined monadic types
        // - Prevention of false positives from coincidental structure

        // ============================================
        // TEMPLATE PATTERN DETECTION
        // ============================================
        // Pattern matching for known monadic types.
        // Note: These are separate from Applicative/Functor traits for
        // independent control over concept satisfaction.

        template<typename T>
        struct is_template_instance_of_vector_monad : std::false_type {};

        template<typename T>
        struct is_template_instance_of_vector_monad<std::vector<T>> : std::true_type {};

        template<typename T>
        struct is_template_instance_of_optional_monad : std::false_type {};

        template<typename T>
        struct is_template_instance_of_optional_monad<std::optional<T>> : std::true_type {};

        // ============================================
        // EXTENSIBILITY MECHANISM FOR MONADS
        // ============================================
        // Custom types can opt-in to being Monads.
        // They must first be Applicatives (and therefore Functors).

        template<typename T>
        struct is_monad_opt_in : std::false_type {};

        // Helper: detect if a type is a known monad via pattern matching
        template<typename T>
        struct is_known_monad : std::bool_constant<
            is_template_instance_of_vector_monad<T>::value ||
            is_template_instance_of_optional_monad<T>::value ||
            is_monad_opt_in<T>::value
        > {};

        // ============================================
        // STRUCTURAL REQUIREMENTS FOR MONADS
        // ============================================
        // A Monad must first be an Applicative, then add monadic operations.
        // The structural check ensures custom types have the necessary interface.
        template<typename M>
        concept HasMonadStructure = requires {
            typename M::value_type;
            requires !std::same_as<M, typename M::value_type>; // Prevent recursion
        } && std::default_initializable<M> && Applicative<M>;

        // ============================================
        // PRIMARY MONAD CONCEPT
        // ============================================
        // The main concept identifying Monads.
        //
        // CRITICAL: The Applicative<M> constraint enables subsumption.
        // This creates the hierarchy: Monad → Applicative → Functor
        template<typename M>
        concept Monad =
            Applicative<M> &&  // SUBSUMPTION: Every Monad is an Applicative
            (is_known_monad<M>::value || HasMonadStructure<M>);
    }

    // ============================================
    // RETURN_ OPERATION - Lifting Values into Monadic Context
    // ============================================
    // return_ :: a → M a
    //
    // Wraps a plain value into a monadic context with minimal effects.
    // Named return_ to avoid conflict with C++ return keyword.
    // Semantically equivalent to pure from Applicative.
    //
    // SEMANTIC MEANING BY TYPE:
    // - std::optional: Creates Some(value)
    // - std::vector: Creates singleton [value]
    // - Result<T,E>: Creates Ok(value)
    // - IO<T>: Creates pure I/O action returning value

    // ============================================
    // RETURN_ FOR STD::OPTIONAL
    // ============================================
    // Creates an optional containing the given value.
    // Time complexity: O(1)
    // Space complexity: O(1)
    template<template<typename...> typename M, typename T>
        requires std::same_as<M<T>, std::optional<T>>
    constexpr std::optional<T> return_(T value) {
        return std::optional<T>{std::move(value)};
    }

    // ============================================
    // RETURN_ FOR STD::VECTOR
    // ============================================
    // Creates a singleton vector containing the given value.
    // Time complexity: O(1)
    // Space complexity: O(1)
    template<template<typename...> typename M, typename T>
        requires std::same_as<M<T>, std::vector<T>>
    std::vector<T> return_(T value) {
        return std::vector<T>{std::move(value)};
    }

    // ============================================
    // BIND OPERATION - Monadic Composition
    // ============================================
    // bind :: M a → (a → M b) → M b
    // Also known as >>= in Haskell or flatMap in other languages.
    //
    // Sequences computations where later effects depend on earlier values.
    // This is the key operation that distinguishes Monads from Applicatives.
    //
    // SEMANTIC MEANING:
    // - Extract value(s) from monadic context
    // - Apply function that returns new monadic context
    // - Flatten/join the nested structure
    //
    // KEY DIFFERENCE FROM APPLICATIVE:
    // - Applicative: Effects are static (known at compile time)
    // - Monad: Effects depend on runtime values

    // ============================================
    // BIND FOR STD::OPTIONAL
    // ============================================
    // Applies function only if value is present, propagates None otherwise.
    // Time complexity: O(1)
    // Space complexity: O(1)
    //
    // Semantic behavior:
    // - Some(x) >>= f → f(x)
    // - None >>= f → None
    template<typename T, typename Func>
    constexpr auto bind(const std::optional<T>& m, Func&& f)
        -> decltype(f(std::declval<T>())) {
        if (m.has_value()) {
            return f(*m);
        }
        return decltype(f(std::declval<T>())){};  // Empty optional of result type
    }

    // ============================================
    // BIND FOR STD::VECTOR
    // ============================================
    // Applies function to each element and flattens the result.
    // This implements the list monad's non-deterministic computation.
    // Time complexity: O(n*m) where n = input size, m = avg output size per element
    // Space complexity: O(n*m)
    //
    // Semantic behavior:
    // - Maps function over each element
    // - Concatenates all resulting vectors
    // - Example: [1,2,3] >>= (\x -> [x, x*2]) → [1,2,2,4,3,6]
    template<typename T, typename Func>
    auto bind(const std::vector<T>& m, Func&& f)
        -> decltype(f(std::declval<T>())) {
        using ResultType = decltype(f(std::declval<T>()));
        ResultType result;

        for (const auto& elem : m) {
            auto partial = f(elem);
            result.insert(result.end(),
                         std::make_move_iterator(partial.begin()),
                         std::make_move_iterator(partial.end()));
        }

        return result;
    }

    // ============================================
    // USAGE EXAMPLES AND PATTERNS
    // ============================================
    // Common monadic usage patterns in functional programming:
    //
    // 1. OPTIONAL CHAINING WITH DATA DEPENDENCY:
    // ```cpp
    // auto result = bind(
    //     find_user(id),
    //     [](const User& u) { return find_account(u.account_id); }
    // );
    // // Chains lookups, propagating None if any step fails
    // ```
    //
    // 2. LIST MONAD FOR NON-DETERMINISTIC COMPUTATION:
    // ```cpp
    // std::vector<int> nums = {1, 2, 3};
    // auto result = bind(nums, [](int x) {
    //     return std::vector<int>{x, -x};  // Each number → two possibilities
    // });
    // // Result: {1, -1, 2, -2, 3, -3}
    // ```
    //
    // 3. ERROR PROPAGATION (with custom Result monad):
    // ```cpp
    // auto result = bind(
    //     parse_int(input),
    //     [](int x) { return safe_divide(100, x); }
    // );
    // // Automatically propagates parse or division errors
    // ```
}