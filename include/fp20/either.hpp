// ============================================
// FP++20: Either<L,R> Monad Implementation
// ============================================
// A right-biased Either monad for error handling and alternative computations.
// Either<L,R> represents a value that is either Left (error/failure) or Right (success).
// All functor/applicative/monad operations are right-biased.
//
// ============================================
// ACADEMIC REFERENCES & FOUNDATIONAL LITERATURE
// ============================================
// Category Theory:
// - Reference: [Coproduct in Category Theory] - https://ncatlab.org/nlab/show/coproduct
// - Reference: [Sum Types] - https://en.wikipedia.org/wiki/Tagged_union
// - Reference: [Category Theory for Programmers] - Bartosz Milewski
//   https://bartoszmilewski.com/2015/01/07/products-and-coproducts/
//
// Functional Programming:
// - Reference: [Haskell Either] - https://hackage.haskell.org/package/base/docs/Data-Either.html
// - Reference: [Scala Either] - https://www.scala-lang.org/api/current/scala/util/Either.html
// - Reference: [Either Monad for Error Handling] - https://wiki.haskell.org/Handling_errors_in_Haskell
//
// C++ Design:
// - Reference: [std::variant] - https://en.cppreference.com/w/cpp/utility/variant
// - Reference: [Monadic Error Handling in C++] - https://www.fpcomplete.com/blog/2012/07/the-either-monad/
//
// ============================================
// MONAD LAWS FOR EITHER
// ============================================
// 1. Left Identity: return a >>= f ≡ f a
//    - bind(Either::right(x), f) == f(x)
//
// 2. Right Identity: m >>= return ≡ m
//    - bind(m, Either::right) == m
//
// 3. Associativity: (m >>= f) >>= g ≡ m >>= (λx -> f x >>= g)
//    - bind(bind(m, f), g) == bind(m, [](auto x) { return bind(f(x), g); })
//
// 4. Error Propagation (Either-specific):
//    - bind(Either::left(e), f) == Either::left(e)  (Left short-circuits)
//    - fmap(f, Either::left(e)) == Either::left(e)  (fmap ignores Left)

#pragma once

#include <variant>
#include <type_traits>
#include <stdexcept>
#include <functional>

namespace fp20 {
    // ============================================
    // EITHER TYPE - Right-Biased Sum Type
    // ============================================
    template<typename L, typename R>
    class Either {
    private:
        std::variant<L, R> value_;

        // Private constructors to enforce factory method usage
        explicit Either(std::variant<L, R> v) : value_(std::move(v)) {}

    public:
        // Type aliases for compatibility with concept checks
        using value_type = R;  // Right-biased: value_type is the Right type
        using error_type = L;

        // Default constructor (required for concepts)
        Either() : value_(L{}) {}

        // ============================================
        // FACTORY METHODS
        // ============================================
        static Either left(L l) {
            return Either{std::variant<L, R>(std::in_place_index<0>, std::move(l))};
        }

        static Either right(R r) {
            return Either{std::variant<L, R>(std::in_place_index<1>, std::move(r))};
        }

        // ============================================
        // PREDICATES
        // ============================================
        bool is_left() const {
            return std::holds_alternative<L>(value_);
        }

        bool is_right() const {
            return std::holds_alternative<R>(value_);
        }

        // ============================================
        // VALUE EXTRACTION (UNSAFE)
        // ============================================
        L left() const {
            if (!is_left()) {
                throw std::runtime_error("Attempted to extract Left from Right Either");
            }
            return std::get<L>(value_);
        }

        R right() const {
            if (!is_right()) {
                throw std::runtime_error("Attempted to extract Right from Left Either");
            }
            return std::get<R>(value_);
        }

        // ============================================
        // PATTERN MATCHING
        // ============================================
        template<typename FL, typename FR>
        auto match(FL&& on_left, FR&& on_right) const
            -> std::common_type_t<
                decltype(on_left(std::declval<L>())),
                decltype(on_right(std::declval<R>()))
            > {
            if (is_left()) {
                return on_left(std::get<L>(value_));
            } else {
                return on_right(std::get<R>(value_));
            }
        }

        template<typename FL, typename FR>
        auto match(FL&& on_left, FR&& on_right)
            -> std::common_type_t<
                decltype(on_left(std::declval<L>())),
                decltype(on_right(std::declval<R>()))
            > {
            if (is_left()) {
                return on_left(std::get<L>(value_));
            } else {
                return on_right(std::get<R>(value_));
            }
        }

        // Internal access for functor/monad operations
        const std::variant<L, R>& value() const { return value_; }
    };

    // ============================================
    // FUNCTOR INSTANCE - Right-Biased fmap
    // ============================================
    template<typename Func, typename L, typename R>
    auto fmap(Func&& f, const Either<L, R>& either) {
        using NewR = std::decay_t<decltype(f(std::declval<R>()))>;

        if (either.is_right()) {
            return Either<L, NewR>::right(f(either.right()));
        }
        return Either<L, NewR>::left(either.left());
    }

    // ============================================
    // APPLICATIVE INSTANCE
    // ============================================

    // pure creates Right
    template<template<typename...> typename F, typename L, typename R>
        requires std::same_as<F<L, R>, Either<L, R>>
    Either<L, R> pure(R value) {
        return Either<L, R>::right(std::move(value));
    }

    // Alternative pure with explicit error type
    template<template<typename...> typename F, typename L>
    auto pure(auto value) -> Either<L, decltype(value)> {
        return Either<L, decltype(value)>::right(std::move(value));
    }

    // apply short-circuits on Left
    template<typename L, typename Func, typename R>
    auto apply(const Either<L, Func>& ef, const Either<L, R>& ex) {
        using ResultType = std::decay_t<decltype(std::declval<Func>()(std::declval<R>()))>;

        // If function is Left, propagate that error
        if (ef.is_left()) {
            return Either<L, ResultType>::left(ef.left());
        }

        // If value is Left, propagate that error
        if (ex.is_left()) {
            return Either<L, ResultType>::left(ex.left());
        }

        // Both are Right, apply function
        return Either<L, ResultType>::right(ef.right()(ex.right()));
    }

    // ============================================
    // MONAD INSTANCE
    // ============================================

    // return_ creates Right (alias for pure)
    template<template<typename...> typename M, typename L, typename R>
        requires std::same_as<M<L, R>, Either<L, R>>
    Either<L, R> return_(R value) {
        return Either<L, R>::right(std::move(value));
    }

    // Alternative return_ with explicit error type
    template<template<typename...> typename M, typename L>
    auto return_(auto value) -> Either<L, decltype(value)> {
        return Either<L, decltype(value)>::right(std::move(value));
    }

    // bind short-circuits on Left
    template<typename L, typename R, typename Func>
    auto bind(const Either<L, R>& m, Func&& f)
        -> decltype(f(std::declval<R>())) {
        using ResultEither = decltype(f(std::declval<R>()));
        using ResultR = typename ResultEither::value_type;

        if (m.is_left()) {
            // Propagate Left with correct result type
            return Either<L, ResultR>::left(m.left());
        }
        return f(m.right());
    }
}

