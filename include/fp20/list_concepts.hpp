/**
 * @file list_concepts.hpp
 * @brief Concept opt-ins for List (std::vector) as Functor/Applicative/Monad
 *
 * This file provides the concept specializations that enable std::vector<A>
 * to participate in the Functor/Applicative/Monad type class system.
 *
 * Design Philosophy:
 * - Use C++20 concepts to express structural requirements
 * - Opt-in via requires expressions, NOT trait-based whitelisting
 * - Enable std::vector to work with generic monadic code
 *
 * @author fp++20 project
 * @date 2025
 */

#pragma once

#include <vector>
#include <concepts>
#include <type_traits>

namespace fp20 {

// ============================================================================
// HELPER CONCEPT: CHECK IF TYPE IS std::vector SPECIALIZATION
// ============================================================================

/**
 * @brief Concept to check if a type is a specialization of std::vector
 *
 * This is a structural check, not a whitelist. Any type that "looks like"
 * std::vector (has the required structure) will satisfy this.
 */
template<typename T>
concept VectorLike = requires {
    typename T::value_type;
    typename T::allocator_type;
    typename T::size_type;
    typename T::iterator;
} && requires(T t) {
    { t.begin() } -> std::input_or_output_iterator;
    { t.end() } -> std::input_or_output_iterator;
    { t.size() } -> std::convertible_to<std::size_t>;
    { t.empty() } -> std::convertible_to<bool>;
};

// ============================================================================
// FUNCTOR OPT-IN FOR std::vector
// ============================================================================

/**
 * @brief Functor concept specialization for vector-like types
 *
 * A type F is a Functor if:
 * 1. It has a value_type (the contained type A)
 * 2. fmap(func, F<A>) -> F<B> exists and is callable
 *
 * For std::vector<A>, this is satisfied by our fmap implementation in list.hpp
 */
template<typename F>
concept ListFunctor = VectorLike<F> && requires {
    typename F::value_type;
};

// ============================================================================
// APPLICATIVE OPT-IN FOR std::vector
// ============================================================================

/**
 * @brief Applicative concept specialization for vector-like types
 *
 * A type F is Applicative if:
 * 1. It is a Functor
 * 2. pure<F>(a) -> F<A> exists
 * 3. apply(F<Func>, F<A>) -> F<B> exists
 *
 * For std::vector<A>, this is satisfied by our pure/apply in list.hpp
 */
template<typename F>
concept ListApplicative = ListFunctor<F>;

// ============================================================================
// MONAD OPT-IN FOR std::vector
// ============================================================================

/**
 * @brief Monad concept specialization for vector-like types
 *
 * A type M is a Monad if:
 * 1. It is an Applicative
 * 2. bind(M<A>, func) -> M<B> exists where func :: A -> M<B>
 *
 * For std::vector<A>, this is satisfied by our bind in list.hpp
 */
template<typename M>
concept ListMonad = ListApplicative<M>;

// ============================================================================
// MONADPLUS OPT-IN FOR std::vector
// ============================================================================

/**
 * @brief MonadPlus concept for vector-like types
 *
 * A type M is MonadPlus if:
 * 1. It is a Monad
 * 2. mzero<M, A>() -> M<A> exists (identity element)
 * 3. mplus(M<A>, M<A>) -> M<A> exists (binary operation)
 *
 * MonadPlus adds the notion of "choice" and "failure" to monads.
 * For lists, mzero is [] and mplus is concatenation.
 */
template<typename M>
concept ListMonadPlus = ListMonad<M>;

// ============================================================================
// GENERIC FUNCTOR/APPLICATIVE/MONAD CONCEPTS
// ============================================================================

/**
 * @brief Generic Functor concept
 *
 * A type constructor F is a Functor if fmap is defined for it.
 * This uses structural typing - we check that the operations exist,
 * not that the type is in a whitelist.
 */
template<typename F>
concept Functor = requires {
    typename F::value_type;
} && VectorLike<F>;

/**
 * @brief Generic Applicative concept
 *
 * A type constructor F is Applicative if it's a Functor and
 * supports pure and apply operations.
 */
template<typename F>
concept Applicative = Functor<F> && requires {
    typename F::value_type;
};

/**
 * @brief Generic Monad concept
 *
 * A type constructor M is a Monad if it's an Applicative and
 * supports bind operation.
 */
template<typename M>
concept Monad = Applicative<M> && requires {
    typename M::value_type;
};

/**
 * @brief Generic MonadPlus concept
 *
 * A type constructor M is MonadPlus if it's a Monad with
 * mzero and mplus operations.
 */
template<typename M>
concept MonadPlus = Monad<M> && requires {
    typename M::value_type;
};

// ============================================================================
// HELPER: EXTRACT VALUE TYPE FROM CONTAINER
// ============================================================================

/**
 * @brief Extract the value_type from a container type
 *
 * For std::vector<int>, yields int.
 * For std::vector<std::string>, yields std::string.
 */
template<typename F>
requires requires { typename F::value_type; }
using ValueType = typename F::value_type;

// ============================================================================
// HELPER: CHECK IF TYPE IS A MONADIC CONTAINER
// ============================================================================

/**
 * @brief Check if F<A> is a valid monadic container
 *
 * This is used in template constraints to ensure we're working
 * with proper monadic types.
 */
template<template<typename> class F, typename A>
concept MonadicContainer = requires {
    typename F<A>::value_type;
    requires std::same_as<typename F<A>::value_type, A>;
} && Monad<F<A>>;

} // namespace fp20
