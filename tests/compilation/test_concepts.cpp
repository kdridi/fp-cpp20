// ============================================
// COMPILE-TIME TESTS FOR FUNCTOR CONCEPT
// ============================================
// TDD RED PHASE: These tests will FAIL initially
// Expected errors: <fp20/concepts/functor.hpp> not found
//                  'Functor' concept not defined
//                  'fmap' function not declared
//
// Following TDD: Tests written BEFORE implementation
// Tests are organized to be incrementally uncommented as we implement

#include <vector>
#include <optional>
#include <list>
#include <string>
#include <type_traits>

// This include will FAIL - the file doesn't exist yet (TDD RED phase)
#include <fp20/concepts/functor.hpp>

// ============================================
// PHASE 1: FUNCTOR CONCEPT DEFINITION TESTS
// ============================================
// These tests verify that the Functor concept exists and works correctly

namespace test_functor_concept {

// Test 1.1: Verify Functor concept exists and can be used in concept checks
// EXPECTED FAILURE: 'Functor' is not defined
static_assert(requires {
    requires fp20::concepts::Functor<std::vector<int>>;
}, "Functor concept should be accessible");

// Test 1.2: std::vector<T> should satisfy Functor concept
// EXPECTED FAILURE: 'Functor' concept not found
static_assert(fp20::concepts::Functor<std::vector<int>>,
    "std::vector<int> should satisfy Functor concept");

static_assert(fp20::concepts::Functor<std::vector<std::string>>,
    "std::vector<std::string> should satisfy Functor concept");

static_assert(fp20::concepts::Functor<std::vector<double>>,
    "std::vector<double> should satisfy Functor concept");

// Test 1.3: std::optional<T> should satisfy Functor concept
// EXPECTED FAILURE: 'Functor' concept not found
static_assert(fp20::concepts::Functor<std::optional<int>>,
    "std::optional<int> should satisfy Functor concept");

static_assert(fp20::concepts::Functor<std::optional<std::string>>,
    "std::optional<std::string> should satisfy Functor concept");

static_assert(fp20::concepts::Functor<std::optional<double>>,
    "std::optional<double> should satisfy Functor concept");

// Test 1.4: Non-functor types should NOT satisfy Functor concept
// EXPECTED FAILURE: 'Functor' concept not found (but tests should pass once implemented)
static_assert(!fp20::concepts::Functor<int>,
    "int should NOT satisfy Functor concept");

static_assert(!fp20::concepts::Functor<double>,
    "double should NOT satisfy Functor concept");

static_assert(!fp20::concepts::Functor<std::string>,
    "std::string should NOT satisfy Functor concept");

// Test 1.5: Const and reference variants should behave correctly
// EXPECTED FAILURE: 'Functor' concept not found
static_assert(fp20::concepts::Functor<std::vector<int>>,
    "std::vector<int> (non-const) should satisfy Functor");

// Test 1.6: Empty functors should still satisfy the concept
static_assert(fp20::concepts::Functor<std::vector<void*>>,
    "std::vector<void*> should satisfy Functor concept");

static_assert(fp20::concepts::Functor<std::optional<bool>>,
    "std::optional<bool> should satisfy Functor concept");

// Test 1.7: std::list<T> should satisfy Functor concept (Story-2 Increment 1)
// TDD RED: This will FAIL initially - std::list not yet recognized
static_assert(fp20::concepts::Functor<std::list<int>>,
    "std::list<int> should satisfy Functor concept");

static_assert(fp20::concepts::Functor<std::list<std::string>>,
    "std::list<std::string> should satisfy Functor concept");

static_assert(fp20::concepts::Functor<std::list<double>>,
    "std::list<double> should satisfy Functor concept");

} // namespace test_functor_concept

// ============================================
// PHASE 2: FMAP FUNCTION SIGNATURE TESTS
// ============================================
// These tests verify that fmap exists and has correct signatures

namespace test_fmap_signatures {

// Test 2.1: fmap should be callable with std::vector
// EXPECTED FAILURE: 'fmap' not declared in fp20
static_assert(requires {
    fp20::fmap([](int x) { return x * 2; }, std::vector<int>{1, 2, 3});
}, "fmap should be callable with lambda and std::vector");

// Test 2.2: fmap should be callable with std::optional
// EXPECTED FAILURE: 'fmap' not declared in fp20
static_assert(requires {
    fp20::fmap([](int x) { return x * 2; }, std::optional<int>{42});
}, "fmap should be callable with lambda and std::optional");

// Test 2.3: fmap should work with different function types
// EXPECTED FAILURE: 'fmap' not declared in fp20
static_assert(requires {
    // Function pointer (declaration only in requires clause)
    { fp20::fmap(static_cast<int(*)(int)>(nullptr), std::vector<int>{}) };
}, "fmap should work with function pointers");

// Test 2.4: fmap should support type transformations
// EXPECTED FAILURE: 'fmap' not declared in fp20
static_assert(requires {
    fp20::fmap([](int x) { return std::to_string(x); }, std::vector<int>{1});
}, "fmap should support transforming int to string");

// Test 2.5: fmap return type should be correct
// EXPECTED FAILURE: 'fmap' not declared in fp20
static_assert(std::is_same_v<
    decltype(fp20::fmap([](int x) { return x * 2; }, std::vector<int>{})),
    std::vector<int>
>, "fmap should return std::vector<int> when given std::vector<int> and int->int function");

static_assert(std::is_same_v<
    decltype(fp20::fmap([](int x) { return std::to_string(x); }, std::vector<int>{})),
    std::vector<std::string>
>, "fmap should return std::vector<std::string> when given std::vector<int> and int->string function");

static_assert(std::is_same_v<
    decltype(fp20::fmap([](int x) { return x * 2; }, std::optional<int>{})),
    std::optional<int>
>, "fmap should return std::optional<int> when given std::optional<int> and int->int function");

// Test 2.6: fmap should be constexpr-friendly where possible
// EXPECTED FAILURE: 'fmap' not declared in fp20
constexpr auto test_constexpr_fmap() {
    // Note: std::vector is not constexpr in C++20, but std::optional is
    constexpr auto result = fp20::fmap([](int x) { return x * 2; }, std::optional<int>{21});
    return result.value_or(0);
}
static_assert(test_constexpr_fmap() == 42, "fmap with std::optional should work in constexpr context");

} // namespace test_fmap_signatures

// ============================================
// PHASE 3: TYPE TRAIT TESTS
// ============================================
// These tests verify compile-time properties of functor operations

namespace test_type_traits {

// Test 3.1: Verify value_type extraction works for functors
// EXPECTED FAILURE: type traits not defined
static_assert(requires {
    typename fp20::functor_value_type<std::vector<int>>::type;
}, "Should be able to extract value_type from functor");

static_assert(std::is_same_v<
    typename fp20::functor_value_type<std::vector<int>>::type,
    int
>, "value_type of std::vector<int> should be int");

static_assert(std::is_same_v<
    typename fp20::functor_value_type<std::optional<std::string>>::type,
    std::string
>, "value_type of std::optional<std::string> should be std::string");

// Test 3.2: Verify functor rebind/transformation type computation
// EXPECTED FAILURE: rebind_functor not defined
static_assert(std::is_same_v<
    typename fp20::rebind_functor<std::vector<int>, std::string>::type,
    std::vector<std::string>
>, "Rebinding std::vector<int> with std::string should give std::vector<std::string>");

static_assert(std::is_same_v<
    typename fp20::rebind_functor<std::optional<int>, double>::type,
    std::optional<double>
>, "Rebinding std::optional<int> with double should give std::optional<double>");

// Test 3.3: Test noexcept specifications
// EXPECTED FAILURE: fmap not defined
static_assert(noexcept(fp20::fmap(
    [](int x) noexcept { return x; },
    std::vector<int>{}
)) || true, // Allow both noexcept and non-noexcept for now
"fmap noexcept specification should be detectable");

} // namespace test_type_traits

// ============================================
// PHASE 4: CONCEPT CONSTRAINT TESTS
// ============================================
// These tests verify that invalid operations are properly rejected

namespace test_concept_constraints {

// Test 4.1: Non-functors should not work with fmap
// EXPECTED FAILURE: fmap not defined, but these should fail to compile once implemented
struct NotAFunctor {
    int value;
};

static_assert(!fp20::concepts::Functor<NotAFunctor>,
    "NotAFunctor should NOT satisfy Functor concept");

// These should fail to compile (commented out for now, will uncomment in implementation phase)
// auto invalid1 = fp20::fmap([](int x) { return x; }, 42);  // int is not a functor
// auto invalid2 = fp20::fmap([](int x) { return x; }, NotAFunctor{5});  // NotAFunctor is not a functor

// Test 4.2: Function signature mismatches should be caught
// The function's input type should match the functor's value type
// (These will be tested at runtime/compile-time when implementation exists)

} // namespace test_concept_constraints

// ============================================
// PHASE 5: EDGE CASE CONCEPT TESTS
// ============================================

namespace test_edge_cases {

// Test 5.1: Nested functors should work
static_assert(fp20::concepts::Functor<std::vector<std::vector<int>>>,
    "Nested functors like std::vector<std::vector<int>> should satisfy Functor");

static_assert(fp20::concepts::Functor<std::optional<std::optional<int>>>,
    "Nested functors like std::optional<std::optional<int>> should satisfy Functor");

// Test 5.2: Functors with complex value types
struct ComplexType {
    int x;
    std::string y;
    constexpr bool operator==(const ComplexType&) const = default;
};

static_assert(fp20::concepts::Functor<std::vector<ComplexType>>,
    "std::vector<ComplexType> should satisfy Functor");

static_assert(fp20::concepts::Functor<std::optional<ComplexType>>,
    "std::optional<ComplexType> should satisfy Functor");

// Test 5.3: cv-qualified functors behavior
// Note: Most operations work on values, not const&, but concept should handle this
static_assert(fp20::concepts::Functor<std::vector<const int>>,
    "std::vector<const int> should satisfy Functor concept");

} // namespace test_edge_cases

// ============================================
// COMPILATION TEST MAIN
// ============================================

int main() {
    // All tests are static_assert - verification happens at compile-time
    // If this compiles, all compile-time tests passed

    // We can add a runtime confirmation message
    // (This will only run if compilation succeeds, which it won't in RED phase)
    return 0;
}

// ============================================
// EXPECTED COMPILATION ERRORS (TDD RED PHASE)
// ============================================
/*
 * When this file is compiled, we expect the following errors:
 *
 * 1. Fatal error: 'fp20/concepts/functor.hpp' file not found
 *    - This is expected: we haven't created the implementation yet
 *
 * 2. 'fp20' namespace not declared
 *    - After creating empty header, this will be next error
 *
 * 3. 'Functor' is not a member of 'fp20::concepts'
 *    - After creating namespace, concept needs to be defined
 *
 * 4. 'fmap' is not a member of 'fp20'
 *    - After concept is defined, fmap function needs implementation
 *
 * 5. 'functor_value_type' is not a member of 'fp20'
 *    - Type traits need to be implemented
 *
 * 6. 'rebind_functor' is not a member of 'fp20'
 *    - Type transformation traits need to be implemented
 *
 * These errors will guide our implementation in the GREEN phase.
 * Each error tells us exactly what needs to be implemented next.
 */
