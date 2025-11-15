// ============================================
// FP++20: Identity<T> Monad Implementation
// ============================================
// A transparent wrapper monad with zero overhead.
// Identity<T> is the trivial monad that performs no effects.
// All operations are transparent pass-throughs with inline/constexpr optimization.
//
// ============================================
// ACADEMIC REFERENCES & FOUNDATIONAL LITERATURE
// ============================================
// Category Theory:
// - Reference: [Identity Functor] - https://ncatlab.org/nlab/show/identity+functor
// - Reference: [Trivial Monad] - https://ncatlab.org/nlab/show/maybe+monad#identity
// - Reference: [Category Theory for Programmers] - Bartosz Milewski
//   https://bartoszmilewski.com/2014/01/14/functors/
//
// Functional Programming:
// - Reference: [Haskell Identity] - https://hackage.haskell.org/package/base/docs/Data-Functor-Identity.html
// - Reference: [Identity Monad] - https://wiki.haskell.org/Identity_monad
// - Reference: [Monad Transformers] - https://en.wikibooks.org/wiki/Haskell/Monad_transformers
//   (Identity is the base case for many monad transformers)
//
// C++ Design:
// - Reference: [Zero-Overhead Abstractions] - https://www.stroustrup.com/abstraction-and-machine.pdf
// - Reference: [constexpr] - https://en.cppreference.com/w/cpp/language/constexpr
// - Reference: [inline] - https://en.cppreference.com/w/cpp/language/inline
//
// ============================================
// MONAD LAWS FOR IDENTITY
// ============================================
// 1. Left Identity: return a >>= f ≡ f a
//    - bind(Identity(x), f) == f(x)
//
// 2. Right Identity: m >>= return ≡ m
//    - bind(m, Identity::Identity) == m
//
// 3. Associativity: (m >>= f) >>= g ≡ m >>= (λx -> f x >>= g)
//    - bind(bind(m, f), g) == bind(m, [](auto x) { return bind(f(x), g); })
//
// 4. Transparency (Identity-specific):
//    - runIdentity(Identity(x)) == x  (Perfect round-trip)
//    - fmap(f, Identity(x)) == Identity(f(x))  (Transparent application)

#pragma once

#include <type_traits>
#include <utility>

namespace fp20 {
    // ============================================
    // IDENTITY TYPE - Transparent Wrapper
    // ============================================
    template<typename T>
    class Identity {
    private:
        T value_;

    public:
        // Type alias for concept compatibility
        using value_type = T;

        // ============================================
        // CONSTRUCTORS
        // ============================================
        // Default constructor (required for concepts)
        constexpr Identity() : value_{} {}

        // Value constructor
        constexpr explicit Identity(T value) : value_(std::move(value)) {}

        // Copy constructor
        constexpr Identity(const Identity&) = default;

        // Move constructor
        constexpr Identity(Identity&&) noexcept = default;

        // Copy assignment
        constexpr Identity& operator=(const Identity&) = default;

        // Move assignment
        constexpr Identity& operator=(Identity&&) noexcept = default;

        // ============================================
        // VALUE EXTRACTION
        // ============================================
        // Extract the wrapped value
        [[nodiscard]] constexpr T runIdentity() const {
            return value_;
        }

        // Non-const version for move extraction
        [[nodiscard]] constexpr T runIdentity() {
            return std::move(value_);
        }

        // Internal access for functor/monad operations
        [[nodiscard]] constexpr const T& value() const& { return value_; }
        [[nodiscard]] constexpr T& value() & { return value_; }
        [[nodiscard]] constexpr T&& value() && { return std::move(value_); }
        [[nodiscard]] constexpr const T&& value() const&& { return std::move(value_); }
    };

    // ============================================
    // FUNCTOR INSTANCE - Direct Application
    // ============================================
    template<typename Func, typename T>
    constexpr auto fmap(Func&& f, const Identity<T>& id) {
        using NewT = std::decay_t<decltype(f(std::declval<T>()))>;
        return Identity<NewT>(f(id.runIdentity()));
    }

    // ============================================
    // APPLICATIVE INSTANCE
    // ============================================

    // pure wraps value in Identity
    template<template<typename...> typename F, typename T>
        requires std::same_as<F<T>, Identity<T>>
    constexpr Identity<T> pure(T value) {
        return Identity<T>(std::move(value));
    }

    // Alternative pure for type deduction
    template<template<typename...> typename F>
    constexpr auto pure(auto value) -> Identity<decltype(value)> {
        return Identity<decltype(value)>(std::move(value));
    }

    // apply unwraps and applies
    template<typename Func, typename T>
    constexpr auto apply(const Identity<Func>& f, const Identity<T>& x) {
        using ResultType = std::decay_t<decltype(std::declval<Func>()(std::declval<T>()))>;
        return Identity<ResultType>(f.runIdentity()(x.runIdentity()));
    }

    // ============================================
    // MONAD INSTANCE
    // ============================================

    // return_ wraps value in Identity
    template<template<typename...> typename M, typename T>
        requires std::same_as<M<T>, Identity<T>>
    constexpr Identity<T> return_(T value) {
        return Identity<T>(std::move(value));
    }

    // Alternative return_ for type deduction
    template<template<typename...> typename M>
    constexpr auto return_(auto value) -> Identity<decltype(value)> {
        return Identity<decltype(value)>(std::move(value));
    }

    // bind is transparent
    template<typename T, typename Func>
    constexpr auto bind(const Identity<T>& m, Func&& f)
        -> decltype(f(std::declval<T>())) {
        return f(m.runIdentity());
    }
}

