/**
 * @file list.hpp
 * @brief List monad implementation for non-deterministic computations
 *
 * The List monad represents non-deterministic computations that can produce
 * multiple results. It models computations that may have zero, one, or many
 * possible outcomes.
 *
 * Monadic Operations:
 * - pure(a) = [a]           (singleton list)
 * - bind(ma, f) = concat(map(f, ma))  (flatMap)
 * - mzero = []              (empty list)
 * - mplus(xs, ys) = xs ++ ys (concatenation)
 *
 * @section references References
 *
 * Academic References:
 * - Wadler, P. (1995). "Monads for functional programming"
 *   In Advanced Functional Programming, Springer LNCS 925
 *   https://homepages.inf.ed.ac.uk/wadler/papers/marktoberdorf/baastad.pdf
 *
 * - Haskell List Monad:
 *   https://hackage.haskell.org/package/base/docs/Control-Monad.html
 *   instance Monad [] where
 *     return x = [x]
 *     xs >>= f = concat (map f xs)
 *
 * - Bird, R., & Wadler, P. (1988). "Introduction to Functional Programming"
 *   Chapter 7: List Comprehensions and the List Monad
 *
 * - The Haskell 98 Report, Section 3.11: List Comprehensions
 *   https://www.haskell.org/onlinereport/exps.html#list-comprehensions
 *
 * @example List Comprehensions via do-notation
 * @code
 * // Pythagorean triples: [(a,b,c) | a <- [1..10], b <- [a..10], c <- [b..10], a^2 + b^2 == c^2]
 * auto triples = bind(range(1, 11), [](int a) {
 *   return bind(range(a, 11), [=](int b) {
 *     return bind(range(b, 11), [=](int c) {
 *       return bind(guard(a*a + b*b == c*c), [=](auto) {
 *         return pure<List>(std::tuple(a, b, c));
 *       });
 *     });
 *   });
 * });
 * @endcode
 *
 * @author fp++20 project
 * @date 2025
 */

#pragma once

#include <vector>
#include <concepts>
#include <algorithm>
#include <ranges>
#include <functional>
#include <iterator>
#include <optional>
#include <stdexcept>
#include "fp20/list_concepts.hpp"

namespace fp20 {

// ============================================================================
// HELPER CONCEPTS AND TYPES
// ============================================================================

/**
 * @brief Unit type for representing "no meaningful value"
 *
 * Used in guard() function and other contexts where we need a value
 * but the value itself carries no information.
 */
struct Unit {
    constexpr bool operator==(const Unit&) const = default;
};

/**
 * @brief Concept to check if a type is a specialization of a template
 *
 * Used to constrain functions that work with std::vector<T> specifically.
 */
template<typename T, template<typename...> class Template>
struct is_specialization_of : std::false_type {};

template<template<typename...> class Template, typename... Args>
struct is_specialization_of<Template<Args...>, Template> : std::true_type {};

template<typename T, template<typename...> class Template>
concept SpecializationOf = is_specialization_of<std::remove_cvref_t<T>, Template>::value;

// ============================================================================
// LIST TYPE ALIAS
// ============================================================================

/**
 * @brief List type alias for std::vector
 *
 * Using std::vector as the underlying representation for List monad.
 * This provides efficient random access, dynamic sizing, and standard
 * library integration.
 *
 * The List monad represents non-deterministic computations where a
 * computation may produce zero, one, or many results.
 */
template<typename A>
using List = std::vector<A>;

// ============================================================================
// NOTE: Functor/Applicative instances are defined in:
// - include/fp20/concepts/functor.hpp (fmap for std::vector)
// - include/fp20/concepts/applicative.hpp (pure/apply for std::vector)
// We don't redefine them here to avoid ambiguity.
// ============================================================================

// ============================================================================
// MONAD INSTANCE
// ============================================================================

/**
 * @brief bind for List (Monad instance)
 *
 * Monadic bind (>>=) for lists. Also known as flatMap.
 *
 * Applies a function returning a list to each element, then flattens
 * the result by concatenating all resulting lists.
 *
 * Pattern: bind([x1, x2, ...], f) = f(x1) ++ f(x2) ++ ...
 *
 * Laws:
 * - Left identity: bind(pure(a), f) = f(a)
 * - Right identity: bind(ma, pure) = ma
 * - Associativity: bind(bind(ma, f), g) = bind(ma, λx. bind(f(x), g))
 *
 * @param list Input list
 * @param func Function from A to List<B>
 * @return Flattened list of results
 */
template<typename A, typename Func>
requires std::invocable<Func, A> &&
         requires(Func f, A a) {
             { std::invoke(f, a) } -> SpecializationOf<std::vector>;
         }
constexpr auto bind(const List<A>& list, Func&& func)
    -> std::invoke_result_t<Func, A>
{
    using ResultList = std::invoke_result_t<Func, A>;
    ResultList result;

    // Pre-calculate approximate size if we can
    // (conservative estimate: at least list.size())
    result.reserve(list.size());

    for (const auto& value : list) {
        auto sublist = std::invoke(func, value);
        result.insert(result.end(),
                     std::make_move_iterator(sublist.begin()),
                     std::make_move_iterator(sublist.end()));
    }

    return result;
}

/**
 * @brief Overload for rvalue list (move optimization)
 */
template<typename A, typename Func>
requires std::invocable<Func, A> &&
         requires(Func f, A a) {
             { std::invoke(f, a) } -> SpecializationOf<std::vector>;
         }
constexpr auto bind(List<A>&& list, Func&& func)
    -> std::invoke_result_t<Func, A>
{
    using ResultList = std::invoke_result_t<Func, A>;
    ResultList result;
    result.reserve(list.size());

    for (auto& value : list) {
        auto sublist = std::invoke(func, std::move(value));
        result.insert(result.end(),
                     std::make_move_iterator(sublist.begin()),
                     std::make_move_iterator(sublist.end()));
    }

    return result;
}

// ============================================================================
// MONADPLUS INSTANCE
// ============================================================================

/**
 * @brief mzero for List (MonadPlus instance)
 *
 * The identity element for mplus - an empty list.
 *
 * Laws:
 * - Left identity: mplus(mzero, xs) = xs
 * - Right identity: mplus(xs, mzero) = xs
 *
 * @return Empty list
 */
template<template<typename> class M, typename A>
requires std::same_as<M<A>, List<A>>
constexpr auto mzero() -> List<A> {
    return List<A>{};
}

/**
 * @brief mplus for List (MonadPlus instance)
 *
 * Concatenates two lists (the monoidal operation).
 *
 * Laws:
 * - Associativity: mplus(mplus(xs, ys), zs) = mplus(xs, mplus(ys, zs))
 * - Left zero: bind(mzero, f) = mzero
 * - Right zero: bind(xs, λ_. mzero) = mzero
 *
 * @param list1 First list
 * @param list2 Second list
 * @return Concatenated list
 */
template<typename A>
constexpr auto mplus(const List<A>& list1, const List<A>& list2) -> List<A> {
    List<A> result;
    result.reserve(list1.size() + list2.size());
    result.insert(result.end(), list1.begin(), list1.end());
    result.insert(result.end(), list2.begin(), list2.end());
    return result;
}

/**
 * @brief Move-optimized mplus overload
 */
template<typename A>
constexpr auto mplus(List<A>&& list1, const List<A>& list2) -> List<A> {
    list1.reserve(list1.size() + list2.size());
    list1.insert(list1.end(), list2.begin(), list2.end());
    return std::move(list1);
}

/**
 * @brief Move-optimized mplus overload
 */
template<typename A>
constexpr auto mplus(const List<A>& list1, List<A>&& list2) -> List<A> {
    List<A> result;
    result.reserve(list1.size() + list2.size());
    result.insert(result.end(), list1.begin(), list1.end());
    result.insert(result.end(),
                 std::make_move_iterator(list2.begin()),
                 std::make_move_iterator(list2.end()));
    return result;
}

// ============================================================================
// GUARD FUNCTION (Essential for List Comprehensions)
// ============================================================================

/**
 * @brief guard for conditional filtering in list comprehensions
 *
 * Used in monadic list comprehensions to filter results based on predicates.
 * Returns a singleton list if condition is true, empty list otherwise.
 *
 * Pattern in Haskell:
 * guard :: Bool -> [()]
 * guard True = [()]
 * guard False = []
 *
 * Usage in comprehensions:
 * @code
 * // [x | x <- [1..10], even x]
 * bind(range(1, 11), [](int x) {
 *   return bind(guard<int>(x % 2 == 0), [=](int) {
 *     return pure<List>(x);
 *   });
 * });
 * @endcode
 *
 * @param condition Predicate to check
 * @return Singleton list with default-constructed value if true, empty list if false
 */
template<typename A>
constexpr auto guard(bool condition) -> List<A> {
    if (condition) {
        return List<A>{A{}};
    } else {
        return List<A>{};
    }
}

/**
 * @brief guard returning Unit (backward compatibility)
 *
 * @param condition Predicate to check
 * @return Singleton [Unit{}] if true, [] if false
 */
constexpr auto guard(bool condition) -> List<Unit> {
    return guard<Unit>(condition);
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * @brief Filter elements satisfying a predicate
 *
 * @param pred Predicate function (A -> bool)
 * @param list Input list
 * @return List of elements satisfying predicate
 */
template<typename A, typename Pred>
requires std::predicate<Pred, A>
constexpr auto filter(Pred&& pred, const List<A>& list) -> List<A> {
    List<A> result;
    std::ranges::copy_if(list, std::back_inserter(result),
                        std::forward<Pred>(pred));
    return result;
}

/**
 * @brief Take first n elements
 *
 * @param n Number of elements to take
 * @param list Input list
 * @return List of first n elements (or entire list if n > size)
 */
template<typename A>
constexpr auto take(std::size_t n, const List<A>& list) -> List<A> {
    auto count = std::min(n, list.size());
    return List<A>(list.begin(), list.begin() + count);
}

/**
 * @brief Drop first n elements
 *
 * @param n Number of elements to drop
 * @param list Input list
 * @return List without first n elements
 */
template<typename A>
constexpr auto drop(std::size_t n, const List<A>& list) -> List<A> {
    auto count = std::min(n, list.size());
    return List<A>(list.begin() + count, list.end());
}

/**
 * @brief Flatten a list of lists (concat/join)
 *
 * Pattern: concat [[1,2], [3,4], [5]] = [1,2,3,4,5]
 *
 * This is equivalent to: bind(lists, id)
 *
 * @param lists List of lists
 * @return Flattened list
 */
template<typename A>
constexpr auto concat(const List<List<A>>& lists) -> List<A> {
    List<A> result;

    // Pre-calculate total size
    std::size_t total_size = 0;
    for (const auto& sublist : lists) {
        total_size += sublist.size();
    }
    result.reserve(total_size);

    for (const auto& sublist : lists) {
        result.insert(result.end(), sublist.begin(), sublist.end());
    }

    return result;
}

/**
 * @brief Generate integer range [start, end)
 *
 * Generates a list of integers from start (inclusive) to end (exclusive).
 *
 * @param start Starting value (inclusive)
 * @param end Ending value (exclusive)
 * @return List of integers in range
 */
constexpr auto range(int start, int end) -> List<int> {
    if (start >= end) {
        return List<int>{};
    }

    List<int> result;
    result.reserve(static_cast<std::size_t>(end - start));

    for (int i = start; i < end; ++i) {
        result.push_back(i);
    }

    return result;
}

/**
 * @brief Generate integer range [0, n)
 *
 * Convenience overload for range starting at 0.
 *
 * @param n Ending value (exclusive)
 * @return List of integers from 0 to n-1
 */
constexpr auto range(int n) -> List<int> {
    return range(0, n);
}

/**
 * @brief Zip two lists into pairs
 *
 * Combines two lists element-wise into pairs.
 * Result has length of shorter list.
 *
 * @param list1 First list
 * @param list2 Second list
 * @return List of pairs
 */
template<typename A, typename B>
constexpr auto zip(const List<A>& list1, const List<B>& list2)
    -> List<std::pair<A, B>>
{
    auto size = std::min(list1.size(), list2.size());
    List<std::pair<A, B>> result;
    result.reserve(size);

    for (std::size_t i = 0; i < size; ++i) {
        result.emplace_back(list1[i], list2[i]);
    }

    return result;
}

/**
 * @brief Replicate a value n times
 *
 * Creates a list with n copies of the given value.
 *
 * @param n Number of copies
 * @param value Value to replicate
 * @return List with n copies of value
 */
template<typename A>
constexpr auto replicate(std::size_t n, const A& value) -> List<A> {
    return List<A>(n, value);
}

/**
 * @brief Reverse a list
 *
 * @param list Input list
 * @return Reversed list
 */
template<typename A>
constexpr auto reverse(const List<A>& list) -> List<A> {
    return List<A>(list.rbegin(), list.rend());
}

/**
 * @brief Get head of list (first element)
 *
 * Returns the first element wrapped in optional for safety.
 * Returns std::nullopt if list is empty.
 *
 * @param list Input list
 * @return Optional containing first element, or nullopt if empty
 */
template<typename A>
constexpr auto head(const List<A>& list) -> std::optional<A> {
    if (list.empty()) {
        return std::nullopt;
    }
    return list.front();
}

/**
 * @brief Get tail of list (all but first element)
 *
 * @param list Input list (must not be empty)
 * @return Tail of list
 * @throws std::out_of_range if list is empty
 */
template<typename A>
constexpr auto tail(const List<A>& list) -> List<A> {
    if (list.empty()) {
        throw std::out_of_range("tail: empty list");
    }
    return List<A>(list.begin() + 1, list.end());
}

/**
 * @brief Check if list is empty
 *
 * @param list Input list
 * @return true if empty, false otherwise
 */
template<typename A>
constexpr auto null(const List<A>& list) -> bool {
    return list.empty();
}

/**
 * @brief Get length of list
 *
 * @param list Input list
 * @return Number of elements
 */
template<typename A>
constexpr auto length(const List<A>& list) -> std::size_t {
    return list.size();
}

// ============================================================================
// ADDITIONAL CONVENIENCE FUNCTIONS
// ============================================================================

/**
 * @brief Create an empty list
 *
 * Convenience function for creating empty lists with explicit type.
 *
 * @return Empty list
 */
template<typename A>
constexpr auto empty() -> List<A> {
    return List<A>{};
}

/**
 * @brief Create a singleton list
 *
 * Creates a list containing exactly one element.
 *
 * @param value The single value to wrap in a list
 * @return Singleton list
 */
template<typename A>
constexpr auto singleton(A value) -> List<A> {
    return List<A>{std::move(value)};
}

/**
 * @brief Create a list from variadic arguments
 *
 * Convenience function for creating lists inline.
 *
 * @param args Values to put in the list
 * @return List containing all arguments
 */
template<typename... Args>
requires (sizeof...(Args) > 0)
constexpr auto list(Args&&... args) -> List<std::common_type_t<std::decay_t<Args>...>> {
    using T = std::common_type_t<std::decay_t<Args>...>;
    return List<T>{std::forward<Args>(args)...};
}

/**
 * @brief Generate integer range [start, end) with step
 *
 * Generates a list of integers from start (inclusive) to end (exclusive)
 * incrementing by step.
 *
 * @param start Starting value (inclusive)
 * @param end Ending value (exclusive)
 * @param step Increment value (must be positive)
 * @return List of integers in range
 */
constexpr auto range(int start, int end, int step) -> List<int> {
    if (step <= 0) {
        throw std::invalid_argument("range: step must be positive");
    }

    if (start >= end) {
        return List<int>{};
    }

    List<int> result;
    // Calculate size
    int count = (end - start + step - 1) / step;
    result.reserve(static_cast<std::size_t>(count));

    for (int i = start; i < end; i += step) {
        result.push_back(i);
    }

    return result;
}

/**
 * @brief Append an element to the end of a list
 *
 * Creates a new list with the element added at the end.
 *
 * @param value Element to append
 * @param list Input list
 * @return New list with element appended
 */
template<typename A>
constexpr auto append(const A& value, const List<A>& list) -> List<A> {
    List<A> result = list;
    result.push_back(value);
    return result;
}

/**
 * @brief Prepend an element to the beginning of a list
 *
 * Creates a new list with the element added at the beginning.
 *
 * @param value Element to prepend
 * @param list Input list
 * @return New list with element prepended
 */
template<typename A>
constexpr auto prepend(const A& value, const List<A>& list) -> List<A> {
    List<A> result;
    result.reserve(list.size() + 1);
    result.push_back(value);
    result.insert(result.end(), list.begin(), list.end());
    return result;
}

/**
 * @brief Zip two lists with a combining function
 *
 * Applies a binary function to corresponding elements from two lists.
 * Result has length of shorter list.
 *
 * @param func Binary function (A, B) -> C
 * @param list1 First list
 * @param list2 Second list
 * @return List of function results
 */
template<typename Func, typename A, typename B>
requires std::invocable<Func, A, B>
constexpr auto zipWith(Func&& func, const List<A>& list1, const List<B>& list2)
    -> List<std::invoke_result_t<Func, A, B>>
{
    using C = std::invoke_result_t<Func, A, B>;
    auto size = std::min(list1.size(), list2.size());
    List<C> result;
    result.reserve(size);

    for (std::size_t i = 0; i < size; ++i) {
        result.push_back(std::invoke(func, list1[i], list2[i]));
    }

    return result;
}

/**
 * @brief Left fold (reduce from left)
 *
 * Combines elements of a list from left to right using a binary function.
 *
 * Pattern: foldl(f, z, [x1, x2, x3]) = f(f(f(z, x1), x2), x3)
 *
 * @param func Binary function (Acc, A) -> Acc
 * @param init Initial accumulator value
 * @param list Input list
 * @return Final accumulated value
 */
template<typename Func, typename Acc, typename A>
requires std::invocable<Func, Acc, A>
constexpr auto foldl(Func&& func, Acc init, const List<A>& list)
    -> std::invoke_result_t<Func, Acc, A>
{
    Acc acc = std::move(init);
    for (const auto& elem : list) {
        acc = std::invoke(func, std::move(acc), elem);
    }
    return acc;
}

/**
 * @brief Right fold (reduce from right)
 *
 * Combines elements of a list from right to left using a binary function.
 *
 * Pattern: foldr(f, z, [x1, x2, x3]) = f(x1, f(x2, f(x3, z)))
 *
 * @param func Binary function (A, Acc) -> Acc
 * @param init Initial accumulator value
 * @param list Input list
 * @return Final accumulated value
 */
template<typename Func, typename Acc, typename A>
requires std::invocable<Func, A, Acc>
constexpr auto foldr(Func&& func, Acc init, const List<A>& list)
    -> std::invoke_result_t<Func, A, Acc>
{
    Acc acc = std::move(init);
    for (auto it = list.rbegin(); it != list.rend(); ++it) {
        acc = std::invoke(func, *it, std::move(acc));
    }
    return acc;
}

/**
 * @brief Sum all elements in a list
 *
 * Requires elements to be addable and default-constructible.
 *
 * @param list Input list
 * @return Sum of all elements
 */
template<typename A>
requires std::default_initializable<A> && requires(A a, A b) { a + b; }
constexpr auto sum(const List<A>& list) -> A {
    return foldl([](A acc, const A& x) { return acc + x; }, A{}, list);
}

/**
 * @brief Product of all elements in a list
 *
 * Requires elements to be multipliable and constructible from 1.
 *
 * @param list Input list
 * @return Product of all elements
 */
template<typename A>
requires std::constructible_from<A, int> && requires(A a, A b) { a * b; }
constexpr auto product(const List<A>& list) -> A {
    return foldl([](A acc, const A& x) { return acc * x; }, A{1}, list);
}

/**
 * @brief Check if element is in list
 *
 * @param value Value to search for
 * @param list Input list
 * @return true if element found, false otherwise
 */
template<typename A>
requires std::equality_comparable<A>
constexpr auto elem(const A& value, const List<A>& list) -> bool {
    return std::ranges::find(list, value) != list.end();
}

/**
 * @brief Find first element matching predicate
 *
 * Returns the first element satisfying the predicate, wrapped in optional.
 *
 * @param pred Predicate function (A -> bool)
 * @param list Input list
 * @return Optional containing first matching element, or nullopt
 */
template<typename Pred, typename A>
requires std::predicate<Pred, A>
constexpr auto find(Pred&& pred, const List<A>& list) -> std::optional<A> {
    auto it = std::ranges::find_if(list, std::forward<Pred>(pred));
    if (it != list.end()) {
        return *it;
    }
    return std::nullopt;
}

/**
 * @brief Check if any element satisfies predicate
 *
 * @param pred Predicate function (A -> bool)
 * @param list Input list
 * @return true if any element matches, false otherwise
 */
template<typename Pred, typename A>
requires std::predicate<Pred, A>
constexpr auto any(Pred&& pred, const List<A>& list) -> bool {
    return std::ranges::any_of(list, std::forward<Pred>(pred));
}

/**
 * @brief Check if all elements satisfy predicate
 *
 * @param pred Predicate function (A -> bool)
 * @param list Input list
 * @return true if all elements match, false otherwise
 */
template<typename Pred, typename A>
requires std::predicate<Pred, A>
constexpr auto all(Pred&& pred, const List<A>& list) -> bool {
    return std::ranges::all_of(list, std::forward<Pred>(pred));
}

// ============================================================================
// OPERATOR OVERLOADS FOR MONADIC COMPOSITION
// ============================================================================

/**
 * @brief Monadic bind operator (>>=)
 *
 * Allows writing: list >>= f instead of bind(list, f)
 *
 * @param list Input list
 * @param func Function A -> List<B>
 * @return Result of bind
 */
template<typename A, typename Func>
requires std::invocable<Func, A> &&
         requires(Func f, A a) {
             { std::invoke(f, a) } -> SpecializationOf<std::vector>;
         }
constexpr auto operator>>=(const List<A>& list, Func&& func)
    -> std::invoke_result_t<Func, A>
{
    return bind(list, std::forward<Func>(func));
}

/**
 * @brief Functor map operator (<$>)
 *
 * Allows writing: f <$> list instead of fmap(f, list)
 *
 * Note: C++ doesn't allow custom operators, so this is a named function
 */
template<typename Func, typename A>
requires std::invocable<Func, A>
constexpr auto operator_fmap(Func&& func, const List<A>& list)
    -> List<std::invoke_result_t<Func, A>>
{
    return fmap(std::forward<Func>(func), list);
}

/**
 * @brief List concatenation operator (+)
 *
 * Implements mplus as operator+ for convenient list concatenation.
 *
 * @param lhs Left list
 * @param rhs Right list
 * @return Concatenated list
 */
template<typename A>
constexpr auto operator+(const List<A>& lhs, const List<A>& rhs) -> List<A> {
    return mplus(lhs, rhs);
}

} // namespace fp20
