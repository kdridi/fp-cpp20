// ============================================
// FP++20: Functor Concept Definition
// ============================================
// TDD GREEN PHASE: Minimal implementation to make tests pass
//
// References:
// - Haskell Functor: https://wiki.haskell.org/Functor
// - Category Theory: https://en.wikipedia.org/wiki/Functor
//
// Functor Laws:
// 1. Identity: fmap id = id
// 2. Composition: fmap (f . g) = fmap f . fmap g

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
        // FUNCTOR CONCEPT
        // ============================================
        // A Functor is a type constructor F that can map a function (A -> B)
        // to a function (F<A> -> F<B>), preserving structure.
        //
        // Structural requirements (C++20):
        // - Must have a value_type member type
        // - Must be default constructible (for creating empty functors)
        // - Must not be a "scalar-like" type (string, string_view, span)
        //
        // This is a proper C++20 concept using requires expressions,
        // replacing the old trait-based approach for modern semantics.

        // ============================================
        // TEMPLATE PATTERN DETECTION (avoiding type instantiation)
        // ============================================
        // These traits detect functor types by template pattern matching,
        // avoiding the need to instantiate potentially ill-formed types
        // (e.g., std::vector<const int> is ill-formed but pattern-matches)

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

        // Type trait for extensibility: types can opt-in via specialization
        template<typename T>
        struct is_functor_opt_in : std::false_type {};

        // Helper: detect if a type is a known functor via pattern matching
        template<typename T>
        struct is_known_functor : std::bool_constant<
            is_template_instance_of_vector<T>::value ||
            is_template_instance_of_optional<T>::value ||
            is_template_instance_of_list<T>::value ||
            is_functor_opt_in<T>::value
        > {};

        // Helper concept: detects scalar-like types that shouldn't be functors
        template<typename T>
        concept ScalarLikeContainer =
            std::same_as<std::remove_cv_t<T>, std::string> ||
            std::same_as<std::remove_cv_t<T>, std::string_view> ||
            std::same_as<std::remove_cv_t<T>, std::wstring> ||
            std::same_as<std::remove_cv_t<T>, std::wstring_view>;

        // Helper: structural requirements for functors (for custom types)
        template<typename F>
        concept HasFunctorStructure = requires {
            typename F::value_type;
            requires !std::same_as<F, typename F::value_type>; // Prevent recursion
        } && std::default_initializable<F>;

        // Primary Functor concept with proper C++20 requires expressions
        // Hybrid approach: pattern matching for known types + structural for custom types
        template<typename F>
        concept Functor =
            !ScalarLikeContainer<F> &&
            (is_known_functor<F>::value || HasFunctorStructure<F>);
    }

    // ============================================
    // TYPE TRAITS
    // ============================================
    // Type-level operations for working with Functors

    // Extract the value_type from a Functor
    // functor_value_type<std::vector<int>>::type == int
    template<typename F>
    struct functor_value_type {
        using type = typename F::value_type;
    };

    // Rebind a Functor with a new value type
    // rebind_functor<std::vector<int>, std::string>::type == std::vector<std::string>
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

    // ============================================
    // FMAP FUNCTION
    // ============================================
    // fmap :: (a -> b) -> F a -> F b
    //
    // Maps a function over a functor, transforming the contained value(s)
    // while preserving the functor's structure.
    //
    // References:
    // - Haskell fmap: https://hackage.haskell.org/package/base/docs/Data-Functor.html

    // fmap for std::vector<T>
    template<typename Func, typename T>
    auto fmap(Func&& f, const std::vector<T>& vec) -> std::vector<decltype(f(std::declval<T>()))> {
        using ResultType = decltype(f(std::declval<T>()));
        std::vector<ResultType> result;
        result.reserve(vec.size());
        for (const auto& elem : vec) {
            result.push_back(f(elem));
        }
        return result;
    }

    // fmap for std::optional<T>
    template<typename Func, typename T>
    constexpr auto fmap(Func&& f, const std::optional<T>& opt) -> std::optional<decltype(f(std::declval<T>()))> {
        using ResultType = decltype(f(std::declval<T>()));
        if (opt.has_value()) {
            return std::optional<ResultType>{f(*opt)};
        }
        return std::nullopt;
    }

    // fmap for std::list<T>
    template<typename Func, typename T>
    auto fmap(Func&& f, const std::list<T>& lst) -> std::list<decltype(f(std::declval<T>()))> {
        using ResultType = decltype(f(std::declval<T>()));
        std::list<ResultType> result;
        for (const auto& elem : lst) {
            result.push_back(f(elem));
        }
        return result;
    }
}
