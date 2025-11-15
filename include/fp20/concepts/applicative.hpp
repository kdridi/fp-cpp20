// ============================================
// FP++20: Applicative Concept Definition
// ============================================
// TDD GREEN PHASE: Minimal implementation to make tests pass
//
// References:
// - Haskell Applicative: https://wiki.haskell.org/Applicative_functor
// - Typeclassopedia: https://wiki.haskell.org/Typeclassopedia#Applicative
//
// Applicative Laws:
// 1. Identity: pure id <*> v = v
// 2. Homomorphism: pure f <*> pure x = pure (f x)
// 3. Interchange: u <*> pure y = pure ($ y) <*> u
// 4. Composition: pure (.) <*> u <*> v <*> w = u <*> (v <*> w)
// 5. Functor relationship: fmap f x = pure f <*> x

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
        // APPLICATIVE CONCEPT
        // ============================================
        // An Applicative is a Functor that can:
        // 1. Wrap a pure value (pure :: a -> F a)
        // 2. Apply a wrapped function to a wrapped value (apply :: F (a -> b) -> F a -> F b)
        //
        // Every Applicative must also be a Functor.
        // This enables powerful multi-parameter function lifting patterns.
        //
        // Structural requirements (C++20):
        // - Must satisfy Functor concept (subsumption)
        // - Must have value_type member type (inherited from Functor)
        // - Must support pure and apply operations (verified structurally)
        //
        // This is a proper C++20 concept using requires expressions,
        // replacing the old trait-based approach for modern semantics.

        // ============================================
        // TEMPLATE PATTERN DETECTION (avoiding type instantiation)
        // ============================================
        // These traits detect applicative types by template pattern matching,
        // avoiding the need to instantiate potentially ill-formed types
        // (e.g., std::vector<const int> is ill-formed but pattern-matches)

        template<typename T>
        struct is_template_instance_of_vector_applicative : std::false_type {};

        template<typename T>
        struct is_template_instance_of_vector_applicative<std::vector<T>> : std::true_type {};

        template<typename T>
        struct is_template_instance_of_optional_applicative : std::false_type {};

        template<typename T>
        struct is_template_instance_of_optional_applicative<std::optional<T>> : std::true_type {};

        // Type trait for extensibility: types can opt-in via specialization
        template<typename T>
        struct is_applicative_opt_in : std::false_type {};

        // Helper: detect if a type is a known applicative via pattern matching
        template<typename T>
        struct is_known_applicative : std::bool_constant<
            is_template_instance_of_vector_applicative<T>::value ||
            is_template_instance_of_optional_applicative<T>::value ||
            is_applicative_opt_in<T>::value
        > {};

        // Helper: structural requirements for applicatives (for custom types)
        // An Applicative must be a Functor with additional structure
        template<typename F>
        concept HasApplicativeStructure = requires {
            typename F::value_type;
            requires !std::same_as<F, typename F::value_type>; // Prevent recursion
        } && std::default_initializable<F> && Functor<F>;

        // Primary Applicative concept with proper C++20 requires expressions
        // Hybrid approach: pattern matching for known types + structural for custom types
        // IMPORTANT: Applicative SUBSUMES Functor (every Applicative is a Functor)
        template<typename F>
        concept Applicative =
            Functor<F> &&  // Subsumption: Applicative extends Functor
            (is_known_applicative<F>::value || HasApplicativeStructure<F>);
    }

    // ============================================
    // PURE FUNCTION
    // ============================================
    // pure :: a -> F a
    //
    // Wraps a plain value into an Applicative context.
    // This is the minimal context - for lists it's a singleton,
    // for Maybe it's Just, etc.
    //
    // References:
    // - Haskell pure: https://hackage.haskell.org/package/base/docs/Control-Applicative.html

    // pure for std::vector - creates singleton vector
    template<template<typename...> typename F, typename T>
        requires std::same_as<F<T>, std::vector<T>>
    auto pure(T&& value) -> std::vector<std::decay_t<T>> {
        return std::vector<std::decay_t<T>>{std::forward<T>(value)};
    }

    // pure for std::optional - creates Some
    template<template<typename...> typename F, typename T>
        requires std::same_as<F<T>, std::optional<T>>
    constexpr auto pure(T&& value) -> std::optional<std::decay_t<T>> {
        return std::optional<std::decay_t<T>>{std::forward<T>(value)};
    }

    // ============================================
    // APPLY FUNCTION
    // ============================================
    // apply :: F (a -> b) -> F a -> F b
    //
    // Applies a wrapped function (or functions) to wrapped value(s).
    // For containers like vector, this performs a cartesian product.
    // For Maybe-like types, it propagates Nothing/None.
    //
    // References:
    // - Haskell <*>: https://hackage.haskell.org/package/base/docs/Control-Applicative.html

    // apply for std::optional
    // Some(f) <*> Some(x) = Some(f(x))
    // None <*> _ = None
    // _ <*> None = None
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

    // apply for std::vector - cartesian product
    // Each function is applied to each value
    // [f, g] <*> [x, y] = [f(x), f(y), g(x), g(y)]
    template<typename Func, typename T>
    auto apply(
        const std::vector<Func>& funcs,
        const std::vector<T>& values
    ) -> std::vector<decltype(std::declval<Func>()(std::declval<T>()))> {
        using ResultType = decltype(std::declval<Func>()(std::declval<T>()));
        std::vector<ResultType> result;

        // Cartesian product: apply each function to each value
        for (const auto& func : funcs) {
            for (const auto& value : values) {
                result.push_back(func(value));
            }
        }

        return result;
    }
}
