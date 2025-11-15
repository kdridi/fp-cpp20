// ============================================
// COMPILE-TIME TESTS FOR APPLICATIVE CONCEPT
// ============================================
// TDD RED PHASE: These tests will FAIL initially
// Expected errors: <fp20/concepts/applicative.hpp> not found
//                  'Applicative' concept not defined
//                  'pure' function not declared
//                  'apply' function not declared
//
// Following TDD: Tests written BEFORE implementation

#include <vector>
#include <optional>
#include <string>
#include <functional>
#include <type_traits>

// This include will FAIL - the file doesn't exist yet (TDD RED phase)
#include <fp20/concepts/applicative.hpp>

// ============================================
// PHASE 1: APPLICATIVE CONCEPT DEFINITION TESTS
// ============================================
// Applicative extends Functor with:
// - pure :: a -> F a (wrap a value)
// - apply :: F (a -> b) -> F a -> F b (apply wrapped function to wrapped value)

namespace test_applicative_concept {

// Test 1.1: Verify Applicative concept exists
// EXPECTED FAILURE: 'Applicative' is not defined
static_assert(requires {
    requires fp20::concepts::Applicative<std::vector<int>>;
}, "Applicative concept should be accessible");

// Test 1.2: std::vector<T> should satisfy Applicative concept
// EXPECTED FAILURE: 'Applicative' concept not found
static_assert(fp20::concepts::Applicative<std::vector<int>>,
    "std::vector<int> should satisfy Applicative concept");

static_assert(fp20::concepts::Applicative<std::vector<std::string>>,
    "std::vector<std::string> should satisfy Applicative concept");

// Test 1.3: std::optional<T> should satisfy Applicative concept
// EXPECTED FAILURE: 'Applicative' concept not found
static_assert(fp20::concepts::Applicative<std::optional<int>>,
    "std::optional<int> should satisfy Applicative concept");

static_assert(fp20::concepts::Applicative<std::optional<std::string>>,
    "std::optional<std::string> should satisfy Applicative concept");

// Test 1.4: Non-applicative types should NOT satisfy Applicative concept
// EXPECTED FAILURE: 'Applicative' concept not found
static_assert(!fp20::concepts::Applicative<int>,
    "int should NOT satisfy Applicative concept");

static_assert(!fp20::concepts::Applicative<std::string>,
    "std::string should NOT satisfy Applicative concept");

// Test 1.5: Applicative extends Functor (every Applicative is a Functor)
// EXPECTED FAILURE: 'Applicative' concept not found
// If something is Applicative, it must also be Functor
static_assert(
    !fp20::concepts::Applicative<std::vector<int>> ||
    fp20::concepts::Functor<std::vector<int>>,
    "Every Applicative must also be a Functor");

} // namespace test_applicative_concept

// ============================================
// PHASE 2: PURE FUNCTION SIGNATURE TESTS
// ============================================
// pure :: a -> F a
// Wraps a plain value into an Applicative context

namespace test_pure_signatures {

// Test 2.1: pure should be callable with int → std::vector<int>
// EXPECTED FAILURE: 'pure' not declared in fp20
static_assert(requires {
    { fp20::pure<std::vector>(42) } -> std::same_as<std::vector<int>>;
}, "pure should wrap int into std::vector<int>");

// Test 2.2: pure should be callable with int → std::optional<int>
// EXPECTED FAILURE: 'pure' not declared in fp20
static_assert(requires {
    { fp20::pure<std::optional>(42) } -> std::same_as<std::optional<int>>;
}, "pure should wrap int into std::optional<int>");

// Test 2.3: pure should work with string → std::vector<std::string>
// EXPECTED FAILURE: 'pure' not declared in fp20
static_assert(requires {
    { fp20::pure<std::vector>(std::string("hello")) } -> std::same_as<std::vector<std::string>>;
}, "pure should wrap string into std::vector<std::string>");

// Test 2.4: pure should be constexpr-friendly for std::optional
// EXPECTED FAILURE: 'pure' not declared in fp20
constexpr auto test_pure_constexpr() {
    constexpr auto result = fp20::pure<std::optional>(42);
    return result.value_or(0);
}
static_assert(test_pure_constexpr() == 42, "pure with std::optional should work in constexpr context");

} // namespace test_pure_signatures

// ============================================
// PHASE 3: APPLY FUNCTION SIGNATURE TESTS
// ============================================
// apply :: F (a -> b) -> F a -> F b
// Applies a wrapped function to a wrapped value

namespace test_apply_signatures {

// Test 3.1: apply should work with std::vector
// EXPECTED FAILURE: 'apply' not declared in fp20
static_assert(requires {
    { fp20::apply(
        std::declval<std::vector<std::function<int(int)>>>(),
        std::declval<std::vector<int>>()
    ) };
}, "apply should be callable with std::vector of functions and values");

// Test 3.2: apply should work with std::optional
// EXPECTED FAILURE: 'apply' not declared in fp20
static_assert(requires {
    { fp20::apply(
        std::declval<std::optional<std::function<int(int)>>>(),
        std::declval<std::optional<int>>()
    ) };
}, "apply should be callable with std::optional of function and value");

// Test 3.3: apply return type should be correct for std::vector
// EXPECTED FAILURE: 'apply' not declared in fp20
static_assert(std::is_same_v<
    decltype(fp20::apply(
        std::vector<std::function<int(int)>>{},
        std::vector<int>{}
    )),
    std::vector<int>
>, "apply should return std::vector<int> for vector of int->int functions");

// Test 3.4: apply should support type transformation
// EXPECTED FAILURE: 'apply' not declared in fp20
static_assert(std::is_same_v<
    decltype(fp20::apply(
        std::vector<std::function<std::string(int)>>{},
        std::vector<int>{}
    )),
    std::vector<std::string>
>, "apply should return std::vector<std::string> for vector of int->string functions");

// Test 3.5: apply should be constexpr-friendly for std::optional
// EXPECTED FAILURE: 'apply' not declared in fp20
constexpr auto test_apply_constexpr() {
    constexpr auto func = std::optional{[](int x) { return x * 2; }};
    constexpr auto value = std::optional{21};
    constexpr auto result = fp20::apply(func, value);
    return result.value_or(0);
}
static_assert(test_apply_constexpr() == 42, "apply with std::optional should work in constexpr context");

} // namespace test_apply_signatures

// ============================================
// EXPECTED COMPILATION ERRORS (TDD RED PHASE)
// ============================================
/*
 * When this file is compiled, we expect the following errors:
 *
 * 1. Fatal error: 'fp20/concepts/applicative.hpp' file not found
 *    - This is expected: we haven't created the implementation yet
 *
 * 2. 'Applicative' is not a member of 'fp20::concepts'
 *    - After creating empty header, concept needs to be defined
 *
 * 3. 'pure' is not a member of 'fp20'
 *    - After concept is defined, pure function needs implementation
 *
 * 4. 'apply' is not a member of 'fp20'
 *    - After pure is defined, apply function needs implementation
 *
 * These errors will guide our implementation in the GREEN phase.
 */
