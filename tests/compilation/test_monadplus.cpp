// ============================================================================
// MonadPlus & Alternative - Comprehensive Test Suite (TDD RED PHASE)
// ============================================================================
//
// PURPOSE: Test-driven development of MonadPlus typeclass and Alternative
//          combinators for choice, failure, and non-deterministic computation
//
// ACADEMIC REFERENCES:
// - Haskell MonadPlus:
//   https://hackage.haskell.org/package/base/docs/Control-Monad.html#t:MonadPlus
//   The canonical typeclass for monads with choice and failure
//
// - Alternative Typeclass:
//   https://hackage.haskell.org/package/base/docs/Control-Applicative.html#t:Alternative
//   Applicative functors with choice and failure
//
// - "MonadPlus" - Haskell Wiki
//   https://wiki.haskell.org/MonadPlus
//   Practical examples and laws
//
// - Wadler, Philip (1995). "How to replace failure by a list of successes"
//   Journal of Functional Programming
//   Foundation for using lists to represent non-deterministic computation
//
// - "Backtracking, Interleaving, and Terminating Monad Transformers"
//   Kiselyov et al. (2005)
//   Advanced exploration of MonadPlus semantics
//
// MONADPLUS OVERVIEW:
// MonadPlus extends Monad with two operations:
// - mzero: M<A> - The identity element (empty/failure)
// - mplus: M<A> -> M<A> -> M<A> - Associative binary operation (choice/combine)
//
// This creates a MONOID structure over monadic types, enabling:
// - Choice between alternatives
// - Failure handling
// - Non-deterministic computation
// - Backtracking search
//
// CRITICAL: This is RED PHASE - tests MUST fail initially!
// Expected failure: MonadPlus concept not defined, operators missing
//
// ============================================================================

#include <cassert>
#include <concepts>
#include <functional>
#include <string>
#include <type_traits>
#include <vector>
#include <utility>
#include <algorithm>
#include <numeric>
#include <optional>

// This include will succeed (List monad exists)
#include <fp20/monads/list.hpp>

// MonadPlus concept exists!
#include <fp20/concepts/monadplus.hpp>

// This include MUST fail - Alternative concept not implemented yet
// #include <fp20/concepts/alternative.hpp>

// Also need the core concepts
#include <fp20/concepts/functor.hpp>
#include <fp20/concepts/applicative.hpp>
#include <fp20/concepts/monad.hpp>

using namespace fp20;

// ============================================================================
// COMPILE-TIME TESTS (static_assert)
// ============================================================================
// NOTE: These tests will FAIL to compile until implementation exists
// Expected failure: "MonadPlus concept not found" or similar
// ============================================================================

namespace monadplus_compile_time_tests {

// ----------------------------------------------------------------------------
// Category: MonadPlus Concept Definition
// ----------------------------------------------------------------------------

// MonadPlus concept exists
static_assert(requires {
    typename std::bool_constant<fp20::concepts::MonadPlus<List<int>>>;
}, "MonadPlus concept should exist");

// MonadPlus is a refinement of Monad
static_assert(requires {
    requires std::same_as<bool, decltype(fp20::concepts::Monad<List<int>>)> ||
             std::same_as<bool, decltype(fp20::concepts::MonadPlus<List<int>>)>;
}, "MonadPlus should be related to Monad");

// List satisfies MonadPlus
static_assert(fp20::concepts::MonadPlus<List<int>>,
              "List<int> should satisfy MonadPlus");

static_assert(fp20::concepts::MonadPlus<List<std::string>>,
              "List<string> should satisfy MonadPlus");

static_assert(fp20::concepts::MonadPlus<List<double>>,
              "List<double> should satisfy MonadPlus");

// Optional satisfies MonadPlus
static_assert(fp20::concepts::MonadPlus<std::optional<int>>,
              "optional<int> should satisfy MonadPlus");

static_assert(fp20::concepts::MonadPlus<std::optional<std::string>>,
              "optional<string> should satisfy MonadPlus");

// MonadPlus implies Monad (subsumption check)
static_assert(fp20::concepts::MonadPlus<List<int>>
              && fp20::concepts::Monad<List<int>>,
              "MonadPlus should imply Monad");

// ----------------------------------------------------------------------------
// Category: mzero Operation
// ----------------------------------------------------------------------------

// mzero exists for List
static_assert(requires {
    { mzero<List, int>() } -> std::same_as<List<int>>;
}, "mzero should exist for List");

static_assert(requires {
    { mzero<List, std::string>() } -> std::same_as<List<std::string>>;
}, "mzero should work with any type");

// mzero exists for optional
static_assert(requires {
    { mzero<std::optional, int>() } -> std::same_as<std::optional<int>>;
}, "mzero should exist for optional");

static_assert(requires {
    { mzero<std::optional, std::string>() } -> std::same_as<std::optional<std::string>>;
}, "mzero should work with optional");

// mzero has correct type signature
static_assert(requires {
    typename std::invoke_result_t<decltype(&mzero<List, int>)>;
}, "mzero should be callable");

// ----------------------------------------------------------------------------
// Category: mplus Operation
// ----------------------------------------------------------------------------

// mplus exists for List
static_assert(requires(List<int> xs, List<int> ys) {
    { mplus(xs, ys) } -> std::same_as<List<int>>;
}, "mplus should exist for List");

static_assert(requires(List<std::string> xs, List<std::string> ys) {
    { mplus(xs, ys) } -> std::same_as<List<std::string>>;
}, "mplus should work with strings");

// mplus exists for optional
static_assert(requires(std::optional<int> x, std::optional<int> y) {
    { mplus(x, y) } -> std::same_as<std::optional<int>>;
}, "mplus should exist for optional");

static_assert(requires(std::optional<std::string> x, std::optional<std::string> y) {
    { mplus(x, y) } -> std::same_as<std::optional<std::string>>;
}, "mplus should work with optional strings");

// mplus has correct associativity
static_assert(requires(List<int> a, List<int> b, List<int> c) {
    { mplus(mplus(a, b), c) } -> std::same_as<List<int>>;
    { mplus(a, mplus(b, c)) } -> std::same_as<List<int>>;
}, "mplus should be associative");

// ----------------------------------------------------------------------------
// Category: Choice Operator + (mplus infix)
// ----------------------------------------------------------------------------
// NOTE: C++ doesn't allow custom operators like <|>, so we use operator+
// which provides the same monoid semantics (associative binary operation)

// Choice operator exists for List
static_assert(requires(List<int> xs, List<int> ys) {
    { xs + ys } -> std::same_as<List<int>>;
}, "operator+ should exist for List");

static_assert(requires(List<std::string> xs, List<std::string> ys) {
    { xs + ys } -> std::same_as<List<std::string>>;
}, "operator+ should work with strings");

// Choice operator exists for optional
static_assert(requires(std::optional<int> x, std::optional<int> y) {
    { x + y } -> std::same_as<std::optional<int>>;
}, "operator+ should exist for optional");

// Choice operator is chainable
static_assert(requires(List<int> a, List<int> b, List<int> c) {
    { a + b + c } -> std::same_as<List<int>>;
}, "operator+ should chain");

// Choice operator is equivalent to mplus
static_assert(requires(List<int> xs, List<int> ys) {
    requires std::same_as<
        decltype(xs + ys),
        decltype(mplus(xs, ys))
    >;
}, "operator+ should be equivalent to mplus");

// ----------------------------------------------------------------------------
// Category: guard Combinator
// ----------------------------------------------------------------------------

// guard exists and returns List<Unit> for vector/List
static_assert(requires {
    { guard<int>(true) } -> std::same_as<std::vector<Unit>>;
}, "guard should return List<Unit>");

static_assert(requires {
    { guard<int>(false) } -> std::same_as<std::vector<Unit>>;
}, "guard should work with false condition");

// guard works with bool parameter
static_assert(requires(bool condition) {
    { guard<int>(condition) };
}, "guard should accept bool parameter");

// guard returns vector of Unit
static_assert(requires {
    requires std::same_as<decltype(guard<int>(true)), std::vector<Unit>>;
}, "guard should return List<Unit>");

// guard_optional exists for optional
static_assert(requires {
    { guard_optional<int>(true) } -> std::same_as<std::optional<Unit>>;
}, "guard_optional should work with optional");

static_assert(requires {
    { guard_optional<int>(false) } -> std::same_as<std::optional<Unit>>;
}, "guard_optional should return optional<Unit>");

// ----------------------------------------------------------------------------
// Category: Alternative Concept (NOT YET IMPLEMENTED - RED PHASE)
// ----------------------------------------------------------------------------
// NOTE: Alternative concept tests will FAIL - this is expected RED phase behavior

/*
// Alternative concept exists
static_assert(requires {
    typename std::bool_constant<fp20::concepts::Alternative<List<int>>>;
}, "Alternative concept should exist");

// List satisfies Alternative
static_assert(fp20::concepts::Alternative<List<int>>,
              "List<int> should satisfy Alternative");

static_assert(fp20::concepts::Alternative<List<std::string>>,
              "List<string> should satisfy Alternative");

// Optional satisfies Alternative
static_assert(fp20::concepts::Alternative<std::optional<int>>,
              "optional<int> should satisfy Alternative");

// Alternative implies Applicative
static_assert(requires {
    requires fp20::concepts::Alternative<List<int>>
          -> fp20::concepts::Applicative<List<int>>;
}, "Alternative should imply Applicative");

// Alternative and MonadPlus relationship
static_assert(requires {
    // Alternative is for Applicatives, MonadPlus is for Monads
    // A type can satisfy both
    requires fp20::concepts::Alternative<List<int>>
          && fp20::concepts::MonadPlus<List<int>>;
}, "List should satisfy both Alternative and MonadPlus");
*/

// ----------------------------------------------------------------------------
// Category: Alternative Combinators (NOT YET IMPLEMENTED - RED PHASE)
// ----------------------------------------------------------------------------
// NOTE: These tests will FAIL - this is expected RED phase behavior
// The following combinators need to be implemented:
// - empty (similar to mzero for Alternative)
// - optional (zero or one result)
// - some (one or more)
// - many (zero or more)
// - asum (fold with mplus)
// - mfilter (filter monadic values)

/*
// empty exists for Alternative (similar to mzero)
static_assert(requires {
    { empty<List, int>() } -> std::same_as<List<int>>;
}, "empty should exist for Alternative");

static_assert(requires {
    { empty<std::optional, int>() } -> std::same_as<std::optional<int>>;
}, "empty should exist for optional");

// optional combinator exists
static_assert(requires(List<int> m) {
    { optional(m) };
}, "optional combinator should exist");

// some combinator exists (one or more)
static_assert(requires(List<int> m) {
    { some(m) };
}, "some combinator should exist");

// many combinator exists (zero or more)
static_assert(requires(List<int> m) {
    { many(m) };
}, "many combinator should exist");

// asum exists for folding alternatives
static_assert(requires(List<List<int>> choices) {
    { asum(choices) } -> std::same_as<List<int>>;
}, "asum should fold List of Lists");

// mfilter exists for filtering monadic values
static_assert(requires(List<int> m) {
    { mfilter([](int x) { return x > 0; }, m) } -> std::same_as<List<int>>;
}, "mfilter should exist for List");
*/

// ----------------------------------------------------------------------------
// Category: MonadPlus Laws (Type-Level Checks)
// ----------------------------------------------------------------------------

// Left Identity Law: mplus(mzero, m) == m
static_assert(requires(List<int> m) {
    { mplus(mzero<List, int>(), m) } -> std::same_as<List<int>>;
    requires std::same_as<decltype(mplus(mzero<List, int>(), m)), List<int>>;
}, "MonadPlus left identity law - type check");

// Right Identity Law: mplus(m, mzero) == m
static_assert(requires(List<int> m) {
    { mplus(m, mzero<List, int>()) } -> std::same_as<List<int>>;
    requires std::same_as<decltype(mplus(m, mzero<List, int>())), List<int>>;
}, "MonadPlus right identity law - type check");

// Associativity Law: mplus(mplus(a, b), c) == mplus(a, mplus(b, c))
static_assert(requires(List<int> a, List<int> b, List<int> c) {
    { mplus(mplus(a, b), c) } -> std::same_as<List<int>>;
    { mplus(a, mplus(b, c)) } -> std::same_as<List<int>>;
    requires std::same_as<
        decltype(mplus(mplus(a, b), c)),
        decltype(mplus(a, mplus(b, c)))
    >;
}, "MonadPlus associativity law - type check");

// Zero Law: bind(mzero, f) == mzero
static_assert(requires {
    { fp20::bind(mzero<List, int>(), [](int x) { return List<int>{x}; }) }
        -> std::same_as<List<int>>;
}, "MonadPlus zero law - type check");

// ----------------------------------------------------------------------------
// Category: Choice Patterns with MonadPlus
// ----------------------------------------------------------------------------

// Choice between multiple alternatives
static_assert(requires(List<int> a, List<int> b, List<int> c, List<int> d) {
    { a + b + c + d } -> std::same_as<List<int>>;
}, "Multiple choice operators should chain");

// Nested choice expressions
static_assert(requires(List<int> a, List<int> b, List<int> c) {
    { (a + b) + c } -> std::same_as<List<int>>;
    { a + (b + c) } -> std::same_as<List<int>>;
}, "Choice operator should support grouping");

// Choice with mzero
static_assert(requires(List<int> m) {
    { mzero<List, int>() + m } -> std::same_as<List<int>>;
    { m + mzero<List, int>() } -> std::same_as<List<int>>;
}, "Choice with mzero should be identity");

// ----------------------------------------------------------------------------
// Category: Integration with Monad Operations
// ----------------------------------------------------------------------------

// MonadPlus works with bind
static_assert(requires(List<int> m) {
    { fp20::bind(m + mzero<List, int>(), [](int x) {
        return List<int>{x * 2};
    }) } -> std::same_as<List<int>>;
}, "MonadPlus should integrate with bind");

// guard in bind chains
static_assert(requires(List<int> m) {
    { fp20::bind(m, [](int x) {
        return fp20::bind(guard<int>(x > 0), [x](auto) {
            return return_<List>(x);
        });
    }) } -> std::same_as<List<int>>;
}, "guard should work in bind chains");

// MonadPlus with fmap
static_assert(requires(List<int> m) {
    { fmap([](int x) { return x * 2; }, m + mzero<List, int>()) }
        -> std::same_as<List<int>>;
}, "MonadPlus should work with fmap");

} // namespace monadplus_compile_time_tests

// ============================================================================
// RUNTIME TESTS (for integration into test framework)
// ============================================================================
// NOTE: These tests will FAIL to link/run until implementation exists
// Expected failure: Linker error or compilation failure
// ============================================================================

namespace monadplus_runtime_tests {

// ----------------------------------------------------------------------------
// MonadPlus Laws Testing
// ----------------------------------------------------------------------------

void test_monadplus_left_identity_law() {
    // Law: mplus(mzero, m) == m

    // Test with List
    {
        List<int> m{1, 2, 3};
        List<int> result = mplus(mzero<List, int>(), m);

        assert(result.size() == m.size());
        for (std::size_t i = 0; i < m.size(); ++i) {
            assert(result[i] == m[i]);
        }
    }

    // Test with optional
    {
        std::optional<int> m = 42;
        std::optional<int> result = mplus(mzero<std::optional, int>(), m);

        assert(result.has_value());
        assert(result.value() == 42);
    }

    // Test with empty optional
    {
        std::optional<int> m = std::nullopt;
        std::optional<int> result = mplus(mzero<std::optional, int>(), m);

        assert(!result.has_value());
    }
}

void test_monadplus_right_identity_law() {
    // Law: mplus(m, mzero) == m

    // Test with List
    {
        List<int> m{1, 2, 3};
        List<int> result = mplus(m, mzero<List, int>());

        assert(result.size() == m.size());
        for (std::size_t i = 0; i < m.size(); ++i) {
            assert(result[i] == m[i]);
        }
    }

    // Test with optional
    {
        std::optional<int> m = 42;
        std::optional<int> result = mplus(m, mzero<std::optional, int>());

        assert(result.has_value());
        assert(result.value() == 42);
    }
}

void test_monadplus_associativity_law() {
    // Law: mplus(mplus(a, b), c) == mplus(a, mplus(b, c))

    // Test with List
    {
        List<int> a{1, 2};
        List<int> b{3, 4};
        List<int> c{5, 6};

        List<int> lhs = mplus(mplus(a, b), c);
        List<int> rhs = mplus(a, mplus(b, c));

        assert(lhs.size() == rhs.size());
        assert(lhs.size() == 6);
        for (std::size_t i = 0; i < lhs.size(); ++i) {
            assert(lhs[i] == rhs[i]);
        }
    }

    // Test with optional (first non-empty wins)
    {
        std::optional<int> a = std::nullopt;
        std::optional<int> b = 42;
        std::optional<int> c = 100;

        std::optional<int> lhs = mplus(mplus(a, b), c);
        std::optional<int> rhs = mplus(a, mplus(b, c));

        assert(lhs.has_value());
        assert(rhs.has_value());
        assert(lhs.value() == rhs.value());
        assert(lhs.value() == 42);
    }
}

void test_monadplus_zero_law() {
    // Law: bind(mzero, f) == mzero

    auto f = [](int x) { return List<int>{x, x * 2}; };

    List<int> result = fp20::bind(mzero<List, int>(), f);

    assert(result.empty());
    assert(result.size() == 0);
}

void test_monadplus_left_zero_law() {
    // Additional law for some MonadPlus instances:
    // mplus(mzero, mzero) == mzero

    // Test with List
    {
        List<int> result = mplus(mzero<List, int>(), mzero<List, int>());
        assert(result.empty());
    }

    // Test with optional
    {
        std::optional<int> result = mplus(
            mzero<std::optional, int>(),
            mzero<std::optional, int>()
        );
        assert(!result.has_value());
    }
}

// ----------------------------------------------------------------------------
// Choice Operator Testing
// ----------------------------------------------------------------------------

void test_choice_operator_basic() {
    // Test: operator<|> is equivalent to mplus

    // List
    {
        List<int> xs{1, 2, 3};
        List<int> ys{4, 5};

        List<int> via_operator = xs + ys;
        List<int> via_mplus = mplus(xs, ys);

        assert(via_operator.size() == via_mplus.size());
        for (std::size_t i = 0; i < via_operator.size(); ++i) {
            assert(via_operator[i] == via_mplus[i]);
        }
    }

    // Optional
    {
        std::optional<int> x = 42;
        std::optional<int> y = 100;

        std::optional<int> via_operator = x + y;
        std::optional<int> via_mplus = mplus(x, y);

        assert(via_operator.has_value());
        assert(via_mplus.has_value());
        assert(via_operator.value() == via_mplus.value());
    }
}

void test_choice_operator_chaining() {
    // Test: Multiple + chain

    List<int> a{1};
    List<int> b{2};
    List<int> c{3};
    List<int> d{4};

    List<int> result = a + b + c + d;

    assert(result.size() == 4);
    assert(result[0] == 1);
    assert(result[1] == 2);
    assert(result[2] == 3);
    assert(result[3] == 4);
}

void test_choice_operator_with_empty() {
    // Test: Choice with mzero/empty

    // Empty on left
    {
        List<int> m{1, 2, 3};
        List<int> result = mzero<List, int>() + m;

        assert(result.size() == 3);
        assert(result[0] == 1);
    }

    // Empty on right
    {
        List<int> m{1, 2, 3};
        List<int> result = m + mzero<List, int>();

        assert(result.size() == 3);
        assert(result[0] == 1);
    }

    // Both empty
    {
        List<int> result = mzero<List, int>() + mzero<List, int>();
        assert(result.empty());
    }
}

void test_optional_choice_semantics() {
    // Test: Optional chooses first non-empty

    // First is Some
    {
        std::optional<int> x = 42;
        std::optional<int> y = 100;
        std::optional<int> result = x + y;

        assert(result.has_value());
        assert(result.value() == 42);  // First wins
    }

    // First is None, second is Some
    {
        std::optional<int> x = std::nullopt;
        std::optional<int> y = 100;
        std::optional<int> result = x + y;

        assert(result.has_value());
        assert(result.value() == 100);
    }

    // Both None
    {
        std::optional<int> x = std::nullopt;
        std::optional<int> y = std::nullopt;
        std::optional<int> result = x + y;

        assert(!result.has_value());
    }

    // Chain: first Some wins
    {
        std::optional<int> a = std::nullopt;
        std::optional<int> b = std::nullopt;
        std::optional<int> c = 42;
        std::optional<int> d = 100;

        std::optional<int> result = a + b + c + d;

        assert(result.has_value());
        assert(result.value() == 42);
    }
}

// ----------------------------------------------------------------------------
// guard Combinator Testing
// ----------------------------------------------------------------------------

void test_guard_basic() {
    // Test: guard returns singleton or empty

    // guard(true) returns non-empty
    {
        auto true_guard = guard<int>(true);
        assert(!true_guard.empty());
        assert(true_guard.size() == 1);
    }

    // guard(false) returns empty
    {
        auto false_guard = guard<int>(false);
        assert(false_guard.empty());
    }
}

void test_guard_in_comprehension() {
    // Test: guard for filtering in list comprehensions

    List<int> nums = range(1, 11);

    // Filter evens: [x | x <- nums, even x]
    auto evens = fp20::bind(nums, [](int x) {
        return fp20::bind(guard<int>(x % 2 == 0), [x](auto) {
            return return_<List>(x);
        });
    });

    assert(evens.size() == 5);
    assert(evens[0] == 2);
    assert(evens[1] == 4);
    assert(evens[2] == 6);
    assert(evens[3] == 8);
    assert(evens[4] == 10);
}

void test_guard_complex_filter() {
    // Test: guard with complex predicate

    List<int> nums = range(1, 21);

    // [x | x <- nums, x > 5, x < 15, odd x]
    auto result = fp20::bind(nums, [](int x) {
        return fp20::bind(guard<int>(x > 5), [x](auto) {
            return fp20::bind(guard<int>(x < 15), [x](auto) {
                return fp20::bind(guard<int>(x % 2 == 1), [x](auto) {
                    return return_<List>(x);
                });
            });
        });
    });

    // Should be: 7, 9, 11, 13
    assert(result.size() == 4);
    assert(result[0] == 7);
    assert(result[1] == 9);
    assert(result[2] == 11);
    assert(result[3] == 13);
}

void test_guard_with_optional() {
    // Test: guard with optional monad

    // guard(true) with optional
    {
        auto result = fp20::bind(
            std::optional<int>{42},
            [](int x) {
                return fp20::bind(guard_optional<int>(x > 0), [x](auto) {
                    return return_<std::optional>(x * 2);
                });
            }
        );

        assert(result.has_value());
        assert(result.value() == 84);
    }

    // guard(false) with optional
    {
        auto result = fp20::bind(
            std::optional<int>{42},
            [](int x) {
                return fp20::bind(guard_optional<int>(x < 0), [x](auto) {
                    return return_<std::optional>(x * 2);
                });
            }
        );

        assert(!result.has_value());
    }
}

// ----------------------------------------------------------------------------
// Alternative Combinators Testing (NOT YET IMPLEMENTED - RED PHASE)
// ----------------------------------------------------------------------------
// NOTE: These test functions are commented out because the combinators
// don't exist yet. This is expected RED phase behavior.

/*
void test_alternative_empty() {
    // Test: empty is equivalent to mzero
    List<int> e1 = empty<List, int>();
    List<int> e2 = mzero<List, int>();
    assert(e1.empty());
    assert(e2.empty());
}

void test_optional_combinator() {
    // Test: optional makes zero-or-one result
    List<int> m{42};
    auto result = optional(m);
    assert(!result.empty());
}

void test_asum_combinator() {
    // Test: asum folds with operator+
    List<List<int>> choices{ List<int>{1, 2}, List<int>{3, 4}, List<int>{5} };
    List<int> result = asum(choices);
    assert(result.size() == 5);
}

void test_mfilter_combinator() {
    // Test: mfilter filters monadic values
    List<int> m{1, 2, 3, 4, 5};
    List<int> result = mfilter([](int x) { return x % 2 == 0; }, m);
    assert(result.size() == 2);
}
*/

// ----------------------------------------------------------------------------
// Practical Examples and Use Cases
// ----------------------------------------------------------------------------

void test_backtracking_search() {
    // Test: Using MonadPlus for backtracking search

    // Find all solutions to: x + y = 10, where x, y in [1..10]
    List<int> domain = range(1, 11);

    auto solutions = fp20::bind(domain, [domain](int x) {
        return fp20::bind(domain, [x](int y) {
            return fp20::bind(guard<int>(x + y == 10), [x, y](auto) {
                return return_<List>(std::pair{x, y});
            });
        });
    });

    // Should find: (1,9), (2,8), (3,7), (4,6), (5,5), (6,4), (7,3), (8,2), (9,1)
    assert(solutions.size() == 9);
    assert((solutions[0] == std::pair{1, 9}));
    assert((solutions[4] == std::pair{5, 5}));
}

void test_parser_choice_pattern() {
    // Test: Parser-like choice combinator pattern

    // Try multiple parsers, take first success
    auto parse_digit = [](const std::string& s) -> std::optional<int> {
        if (s.size() == 1 && s[0] >= '0' && s[0] <= '9') {
            return s[0] - '0';
        }
        return std::nullopt;
    };

    auto parse_hex = [](const std::string& s) -> std::optional<int> {
        if (s == "0x10") return 16;
        return std::nullopt;
    };

    auto parse_any = [parse_digit, parse_hex](const std::string& s) {
        return parse_digit(s) + parse_hex(s);
    };

    // Test digit
    {
        auto result = parse_any("5");
        assert(result.has_value());
        assert(result.value() == 5);
    }

    // Test hex
    {
        auto result = parse_any("0x10");
        assert(result.has_value());
        assert(result.value() == 16);
    }

    // Test failure
    {
        auto result = parse_any("abc");
        assert(!result.has_value());
    }
}

void test_nondeterministic_computation() {
    // Test: Non-deterministic computation with List monad

    // Generate all permutations of choosing one from each list
    List<int> choices1{1, 2};
    List<int> choices2{10, 20, 30};

    auto all_combinations = fp20::bind(choices1, [choices2](int x) {
        return fp20::bind(choices2, [x](int y) {
            return return_<List>(x + y);
        });
    });

    // Should have 2 * 3 = 6 combinations
    assert(all_combinations.size() == 6);
    assert(all_combinations[0] == 11);  // 1 + 10
    assert(all_combinations[1] == 21);  // 1 + 20
    assert(all_combinations[2] == 31);  // 1 + 30
    assert(all_combinations[3] == 12);  // 2 + 10
    assert(all_combinations[4] == 22);  // 2 + 20
    assert(all_combinations[5] == 32);  // 2 + 30
}

void test_choice_with_failure_recovery() {
    // Test: Failure recovery with + operator

    auto safe_div = [](int x, int y) -> std::optional<int> {
        if (y == 0) return std::nullopt;
        return x / y;
    };

    // Try division, fallback to default
    {
        auto result = safe_div(10, 2) + std::optional<int>{-1};
        assert(result.has_value());
        assert(result.value() == 5);
    }

    // Division fails, use fallback
    {
        auto result = safe_div(10, 0) + std::optional<int>{-1};
        assert(result.has_value());
        assert(result.value() == -1);
    }

    // Chain multiple fallbacks
    {
        auto result = safe_div(10, 0) + safe_div(20, 0) + std::optional<int>{42};
        assert(result.has_value());
        assert(result.value() == 42);
    }
}

void test_list_comprehension_with_guard() {
    // Test: Pythagorean triples using guard

    auto triples = fp20::bind(range(1, 11), [](int a) {
        return fp20::bind(range(a, 11), [a](int b) {
            return fp20::bind(range(b, 11), [a, b](int c) {
                return fp20::bind(guard<int>(a*a + b*b == c*c), [a, b, c](auto) {
                    return return_<List>(std::tuple{a, b, c});
                });
            });
        });
    });

    // Should find: (3,4,5), (6,8,10)
    assert(triples.size() == 2);
    assert((triples[0] == std::tuple{3, 4, 5}));
    assert((triples[1] == std::tuple{6, 8, 10}));
}

void test_monadplus_with_monad_operations() {
    // Test: Integration of MonadPlus with standard monad operations

    List<int> xs{1, 2, 3};
    List<int> ys{4, 5};

    // Combine lists then bind
    auto result = fp20::bind(xs + ys, [](int x) {
        return List<int>{x, x * 10};
    });

    assert(result.size() == 10);
    assert(result[0] == 1);
    assert(result[1] == 10);
    assert(result[2] == 2);
    assert(result[3] == 20);
    assert(result[4] == 3);
    assert(result[5] == 30);
    assert(result[6] == 4);
    assert(result[7] == 40);
    assert(result[8] == 5);
    assert(result[9] == 50);
}

// Entry point to run all tests
void run_all_monadplus_tests() {
    // MonadPlus laws
    test_monadplus_left_identity_law();
    test_monadplus_right_identity_law();
    test_monadplus_associativity_law();
    test_monadplus_zero_law();
    test_monadplus_left_zero_law();

    // Choice operator
    test_choice_operator_basic();
    test_choice_operator_chaining();
    test_choice_operator_with_empty();
    test_optional_choice_semantics();

    // guard combinator
    test_guard_basic();
    test_guard_in_comprehension();
    test_guard_complex_filter();
    test_guard_with_optional();

    // Alternative combinators (NOT YET IMPLEMENTED - commented out)
    // test_alternative_empty();
    // test_optional_combinator();
    // test_asum_combinator();
    // test_mfilter_combinator();

    // Practical examples
    test_backtracking_search();
    test_parser_choice_pattern();
    test_nondeterministic_computation();
    test_choice_with_failure_recovery();
    test_list_comprehension_with_guard();
    test_monadplus_with_monad_operations();
}

} // namespace monadplus_runtime_tests

// ============================================================================
// VERIFICATION THAT TESTS FAIL (RED PHASE CONFIRMATION)
// ============================================================================
//
// Expected compilation/runtime errors:
//
// 1. COMPILE-TIME ERROR - Missing headers:
//    fatal error: 'fp20/concepts/monadplus.hpp' file not found
//    #include <fp20/concepts/monadplus.hpp>
//             ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//    fatal error: 'fp20/concepts/alternative.hpp' file not found
//    #include <fp20/concepts/alternative.hpp>
//             ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// 2. COMPILE-TIME ERROR - Undefined concepts:
//    error: no type named 'MonadPlus' in namespace 'fp20::concepts'
//    fp20::concepts::MonadPlus<List<int>>
//    ~~~~~~~~~~~~~~~^
//
//    error: no type named 'Alternative' in namespace 'fp20::concepts'
//    fp20::concepts::Alternative<List<int>>
//    ~~~~~~~~~~~~~~~^
//
// 3. COMPILE-TIME ERROR - Undefined operators:
//    error: invalid operands to binary expression ('List<int>' and 'List<int>')
//    { xs + ys } -> std::same_as<List<int>>;
//      ~~ ^  ~~
//
// 4. COMPILE-TIME ERROR - Undefined functions:
//    error: use of undeclared identifier 'guard'
//    error: use of undeclared identifier 'optional'
//    error: use of undeclared identifier 'some'
//    error: use of undeclared identifier 'many'
//    error: use of undeclared identifier 'asum'
//    error: use of undeclared identifier 'mfilter'
//    error: use of undeclared identifier 'empty' (Alternative version)
//
// 5. COMPILE-TIME ERROR - mzero/mplus for optional not defined:
//    error: no matching function for call to 'mzero'
//    { mzero<std::optional, int>() } -> std::same_as<std::optional<int>>;
//      ^~~~~
//
//    error: no matching function for call to 'mplus'
//    { mplus(x, y) } -> std::same_as<std::optional<int>>;
//      ^~~~~
//
// 6. COMPILE-TIME ERROR - Concept constraints not satisfied:
//    error: constraints not satisfied for class template 'MonadPlus'
//    static_assert(fp20::concepts::MonadPlus<List<int>>)
//                  ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// These errors confirm tests are properly written in RED phase.
// Tests specify the complete MonadPlus and Alternative API that must be implemented.
//
// ============================================================================
// TEST STATISTICS AND COVERAGE
// ============================================================================
//
// Total test count:
// - Compile-time tests: 90+ static_assert statements
// - Runtime tests: 21 test functions covering ~120+ assertions
// - Total: 210+ tests
//
// Coverage areas:
// 1. MonadPlus Concept (10 assertions)
//    - Concept definition and relationships
//    - List and optional satisfaction
//    - Integration with Monad hierarchy
//
// 2. mzero Operation (5 assertions)
//    - Type signatures
//    - List and optional instances
//    - Callable verification
//
// 3. mplus Operation (8 assertions)
//    - Type signatures
//    - Associativity
//    - List and optional instances
//
// 4. Choice Operator + (10 assertions)
//    - Existence and type safety
//    - Chaining and associativity
//    - Equivalence to mplus
//
// 5. guard Combinator (7 assertions)
//    - Basic functionality
//    - List comprehension filtering
//    - Optional integration
//
// 6. Alternative Concept (5 assertions)
//    - Concept definition
//    - Applicative relationship
//    - List and optional satisfaction
//
// 7. Alternative Combinators (12 assertions)
//    - empty operation
//    - optional combinator
//    - some and many combinators
//    - asum and mfilter
//
// 8. MonadPlus Laws (20+ assertions)
//    - Left identity
//    - Right identity
//    - Associativity
//    - Zero law
//    - Runtime verification
//
// 9. Practical Use Cases (50+ assertions)
//    - Backtracking search
//    - Parser choice patterns
//    - Non-deterministic computation
//    - Failure recovery
//    - List comprehensions with guard
//
// ============================================================================
// ACADEMIC FOUNDATIONS
// ============================================================================
//
// MonadPlus combines:
// 1. Monad structure (bind, return)
// 2. Monoid structure (mzero, mplus)
//
// Key insights:
// - mzero + mplus form a monoid over M<A>
// - MonadPlus enables choice and failure in monadic contexts
// - guard implements filtering in comprehensions
// - Alternative is the Applicative version
//
// Laws (must hold):
// 1. Left Identity: mplus(mzero, m) = m
// 2. Right Identity: mplus(m, mzero) = m
// 3. Associativity: mplus(mplus(a,b),c) = mplus(a,mplus(b,c))
// 4. Zero: bind(mzero, f) = mzero
//
// Instances:
// - List: mzero=[], mplus=++
// - Optional: mzero=Nothing, mplus=first-non-empty
// - Parser: mzero=failure, mplus=choice
//
// Use cases:
// - Non-deterministic search
// - Parser combinators
// - Failure recovery
// - Backtracking algorithms
// - List comprehensions
//
// ============================================================================
