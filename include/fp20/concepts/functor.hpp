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

namespace fp20 {
    namespace concepts {
        // ============================================
        // FUNCTOR CONCEPT
        // ============================================
        // A Functor is a type constructor F that can map a function (A -> B)
        // to a function (F<A> -> F<B>), preserving structure.
        //
        // Minimal requirements:
        // - Must be a template type
        // - Must have a value_type member
        // - Must not be a scalar-like type (e.g., std::string is a container of char,
        //   but we treat it as a scalar value in FP context)

        template<typename T>
        struct is_functor_type : std::false_type {};

        template<typename T>
        struct is_functor_type<std::vector<T>> : std::true_type {};

        template<typename T>
        struct is_functor_type<std::optional<T>> : std::true_type {};

        template<typename F>
        concept Functor = is_functor_type<F>::value;
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
}
