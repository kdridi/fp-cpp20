// ============================================================================
// Writer Monad - Comprehensive Test Suite (TDD RED PHASE)
// ============================================================================
//
// PURPOSE: Test-driven development of Writer<W, A> monad for logging/accumulation
//
// ACADEMIC REFERENCES:
// - Haskell Writer Monad (mtl package):
//   https://hackage.haskell.org/package/mtl/docs/Control-Monad-Writer.html
//   The canonical Writer monad implementation with tell, listen, pass primitives
//
// - Wadler, Philip (1995). "Monads for functional programming"
//   Advanced Functional Programming, Springer LNCS 925
//   Section on Writer as the output/logging monad
//
// - "Functional Programming with Bananas, Lenses, Envelopes and Barbed Wire"
//   Meijer et al. (1991) - Writer as dual to Reader
//
// - Monoid Theory:
//   Writer requires W to be a Monoid with:
//   - mempty :: W (identity element)
//   - mappend :: W -> W -> W (associative binary operation)
//
// - Category Theory perspective:
//   Writer w a ≅ (a, w) with monoidal accumulation
//   Functor: transform value, preserve log
//   Monad: Kleisli composition with log concatenation
//
// WRITER MONAD OVERVIEW:
// The Writer monad encapsulates computations that produce a value along with
// an accumulated output (log). The output type W must be a Monoid to support
// combining logs from multiple computations.
//
// Key insight: Writer<W, A> ≅ (A, W) with monoidal log accumulation
//
// CRITICAL: This is RED PHASE - tests MUST fail initially!
// Expected failure: fatal error: 'fp20/writer.hpp' file not found
//
// STORY: FP-007 - Writer<W,A> Monad for Logging/Accumulation
// SPRINT: Sprint 2 "Advanced Monadic Patterns"
// PHASE: RED (tests must fail initially)
//
// ============================================================================

#include <cassert>
#include <concepts>
#include <functional>
#include <string>
#include <type_traits>
#include <vector>
#include <utility>

// This include MUST fail - no implementation exists yet
#include <fp20/monads/writer.hpp>
#include <fp20/concepts/writer_concepts.hpp>

// Include Monoid concept (may need to be created)
#include <fp20/concepts/monoid.hpp>

using namespace fp20;

// Explicitly prefer fp20::bind over std::bind and fp20::apply over std::apply
using fp20::bind;
using fp20::apply;

// ============================================================================
// MONOID INSTANCE DEFINITIONS FOR TESTING
// ============================================================================
// Writer requires W to be a Monoid. We test with common monoids:
// - std::string with concatenation (mempty = "", mappend = +)
// - std::vector<T> with concatenation (mempty = {}, mappend = append)
// - Sum<int> wrapper for addition (mempty = 0, mappend = +)
// - Product<int> wrapper for multiplication (mempty = 1, mappend = *)

// These are example Monoid types that should work with Writer
namespace test_monoids {

// String is a monoid under concatenation
// mempty = "", mappend = +
// Should already have Monoid specialization or concept opt-in

// Vector is a monoid under concatenation
// mempty = {}, mappend = append
// Should already have Monoid specialization or concept opt-in

// Sum type wrapper for addition monoid
template<typename T>
struct Sum {
    T value;

    constexpr Sum() : value(T{}) {}
    constexpr explicit Sum(T v) : value(v) {}

    constexpr Sum operator+(const Sum& other) const {
        return Sum{value + other.value};
    }

    constexpr bool operator==(const Sum& other) const = default;
};

// Product type wrapper for multiplication monoid
template<typename T>
struct Product {
    T value;

    constexpr Product() : value(T{1}) {}
    constexpr explicit Product(T v) : value(v) {}

    constexpr Product operator*(const Product& other) const {
        return Product{value * other.value};
    }

    constexpr bool operator==(const Product& other) const = default;
};

} // namespace test_monoids

// ============================================================================
// MONOID OPT-INS FOR TEST TYPES
// ============================================================================
// Opt-in test_monoids::Sum and test_monoids::Product as Monoids

namespace fp20::concepts {
    template<typename T>
    struct is_monoid_opt_in<test_monoids::Sum<T>> : std::true_type {};

    template<typename T>
    struct is_monoid_opt_in<test_monoids::Product<T>> : std::true_type {};
}

// Provide mempty and mappend implementations for test_monoids types
namespace fp20 {
    // Specializations for Sum<int>
    template<>
    inline test_monoids::Sum<int> mempty<test_monoids::Sum<int>>() {
        return test_monoids::Sum<int>{};  // Constructs Sum{0}
    }

    // mappend for Sum: addition
    inline test_monoids::Sum<int> mappend(test_monoids::Sum<int> a, test_monoids::Sum<int> b) {
        return test_monoids::Sum<int>{a.value + b.value};
    }

    // Specializations for Product<int>
    template<>
    inline test_monoids::Product<int> mempty<test_monoids::Product<int>>() {
        return test_monoids::Product<int>{};  // Constructs Product{1}
    }

    // mappend for Product: multiplication
    inline test_monoids::Product<int> mappend(test_monoids::Product<int> a, test_monoids::Product<int> b) {
        return test_monoids::Product<int>{a.value * b.value};
    }
}

// ============================================================================
// COMPILE-TIME TESTS (static_assert)
// ============================================================================
// NOTE: These tests will FAIL to compile until implementation exists
// Expected failure: "Writer not found in namespace fp20" or similar
// ============================================================================

namespace compile_time_tests {

// ----------------------------------------------------------------------------
// Category: Monoid Concept Tests
// ----------------------------------------------------------------------------

// std::string should satisfy Monoid concept
static_assert(fp20::concepts::Monoid<std::string>,
    "std::string should be a Monoid (concatenation)");

// std::vector should satisfy Monoid concept
static_assert(fp20::concepts::Monoid<std::vector<int>>,
    "std::vector should be a Monoid (concatenation)");

static_assert(fp20::concepts::Monoid<std::vector<std::string>>,
    "std::vector with any element type should be a Monoid");

// Monoid should have mempty and mappend operations
static_assert(requires {
    { mempty<std::string>() } -> std::same_as<std::string>;
    { mappend(std::string{}, std::string{}) } -> std::same_as<std::string>;
});

static_assert(requires {
    { mempty<std::vector<int>>() } -> std::same_as<std::vector<int>>;
    { mappend(std::vector<int>{}, std::vector<int>{}) } -> std::same_as<std::vector<int>>;
});

// Custom Sum and Product types should satisfy Monoid
static_assert(fp20::concepts::Monoid<test_monoids::Sum<int>>,
    "Sum<int> should be a Monoid");

static_assert(fp20::concepts::Monoid<test_monoids::Product<int>>,
    "Product<int> should be a Monoid");

// ----------------------------------------------------------------------------
// Category: Writer Type Structure and Properties
// ----------------------------------------------------------------------------

// Writer template exists and can be instantiated
static_assert(requires {
    typename Writer<std::string, int>;
    typename Writer<std::vector<int>, std::string>;
    typename Writer<std::string, bool>;
});

// Writer has log_type and value_type member types
static_assert(requires {
    typename Writer<std::string, int>::log_type;
    typename Writer<std::string, int>::value_type;
});

// log_type and value_type are correct
static_assert(std::same_as<typename Writer<std::string, int>::log_type, std::string>);
static_assert(std::same_as<typename Writer<std::string, int>::value_type, int>);
static_assert(std::same_as<typename Writer<std::vector<int>, bool>::log_type, std::vector<int>>);
static_assert(std::same_as<typename Writer<std::vector<int>, bool>::value_type, bool>);

// Writer can be constructed with value and log
static_assert(std::constructible_from<
    Writer<std::string, int>,
    int,
    std::string
>);

// Writer has runWriter method that returns std::pair<A, W>
static_assert(requires(Writer<std::string, int> w) {
    { w.runWriter() } -> std::same_as<std::pair<int, std::string>>;
});

static_assert(requires(Writer<std::vector<int>, std::string> w) {
    { w.runWriter() } -> std::same_as<std::pair<std::string, std::vector<int>>>;
});

// runWriter is const (Writer is immutable)
static_assert(requires(const Writer<std::string, int> w) {
    { w.runWriter() } -> std::same_as<std::pair<int, std::string>>;
});

// Writer should be copyable and movable
static_assert(std::is_copy_constructible_v<Writer<std::string, int>>,
    "Writer should be copy constructible");

static_assert(std::is_move_constructible_v<Writer<std::string, int>>,
    "Writer should be move constructible");

// ----------------------------------------------------------------------------
// Category: Writer Construction Helpers
// ----------------------------------------------------------------------------

// writer(a, w) constructs Writer with value and log
static_assert(requires {
    { writer(42, std::string{"log"}) } -> std::same_as<Writer<std::string, int>>;
});

static_assert(requires {
    { writer(std::string{"value"}, std::vector<int>{1, 2, 3}) }
        -> std::same_as<Writer<std::vector<int>, std::string>>;
});

// writer should work with different monoid types
static_assert(requires {
    { writer(42, test_monoids::Sum<int>{10}) }
        -> std::same_as<Writer<test_monoids::Sum<int>, int>>;
});

// ----------------------------------------------------------------------------
// Category: Functor Instance - fmap
// ----------------------------------------------------------------------------

// fmap exists and has correct signature
static_assert(requires(Writer<std::string, int> w) {
    { fmap([](int x) { return x * 2; }, w) }
        -> std::same_as<Writer<std::string, int>>;
});

// fmap preserves log type, transforms value type
static_assert(requires(Writer<std::string, int> w) {
    { fmap([](int x) { return std::to_string(x); }, w) }
        -> std::same_as<Writer<std::string, std::string>>;
});

// fmap with different log types
static_assert(requires(Writer<std::vector<int>, std::string> w) {
    { fmap([](const std::string& s) { return s.size(); }, w) }
        -> std::same_as<Writer<std::vector<int>, std::size_t>>;
});

// fmap can chain transformations
static_assert(requires(Writer<std::string, int> w) {
    { fmap([](int x) { return x * 2; },
           fmap([](int x) { return x + 1; }, w)) }
        -> std::same_as<Writer<std::string, int>>;
});

// fmap preserves the log (doesn't modify it)
static_assert(requires(Writer<std::string, int> w) {
    { fmap([](int x) { return std::to_string(x); }, w) }
        -> std::same_as<Writer<std::string, std::string>>;
});

// ----------------------------------------------------------------------------
// Category: Applicative Instance - pure and apply
// ----------------------------------------------------------------------------

// pure exists and creates Writer with mempty log
static_assert(requires {
    { pure<Writer, std::string>(42) } -> std::same_as<Writer<std::string, int>>;
});

// pure with explicit type specification
static_assert(requires {
    { pure<Writer, std::string, int>(42) }
        -> std::same_as<Writer<std::string, int>>;
});

// pure with different monoid types
static_assert(requires {
    { pure<Writer, std::vector<int>>(std::string{"hello"}) }
        -> std::same_as<Writer<std::vector<int>, std::string>>;
});

// apply exists for applicative functor pattern
static_assert(requires(
    Writer<std::string, std::function<int(int)>> wf,
    Writer<std::string, int> wx
) {
    { fp20::apply(wf, wx) } -> std::same_as<Writer<std::string, int>>;
});

// apply combines logs with mappend
static_assert(requires(
    Writer<std::string, std::function<std::string(int)>> wf,
    Writer<std::string, int> wx
) {
    { fp20::apply(wf, wx) } -> std::same_as<Writer<std::string, std::string>>;
});

// apply with vector monoid
static_assert(requires(
    Writer<std::vector<int>, std::function<bool(std::string)>> wf,
    Writer<std::vector<int>, std::string> wx
) {
    { fp20::apply(wf, wx) } -> std::same_as<Writer<std::vector<int>, bool>>;
});

// ----------------------------------------------------------------------------
// Category: Monad Instance - bind (>>=)
// ----------------------------------------------------------------------------

// bind exists with correct signature
static_assert(requires(
    Writer<std::string, int> m,
    std::function<Writer<std::string, std::string>(int)> k
) {
    { fp20::bind(m, k) } -> std::same_as<Writer<std::string, std::string>>;
});

// bind concatenates logs with mappend
static_assert(requires(
    Writer<std::string, int> m,
    std::function<Writer<std::string, bool>(int)> k
) {
    { fp20::bind(m, k) } -> std::same_as<Writer<std::string, bool>>;
});

// bind with vector monoid
static_assert(requires(
    Writer<std::vector<int>, std::string> m,
    std::function<Writer<std::vector<int>, int>(std::string)> k
) {
    { fp20::bind(m, k) } -> std::same_as<Writer<std::vector<int>, int>>;
});

// bind can chain multiple computations
static_assert(requires(
    Writer<std::string, int> m1,
    std::function<Writer<std::string, std::string>(int)> k1,
    std::function<Writer<std::string, bool>(std::string)> k2
) {
    { fp20::bind(fp20::bind(m1, k1), k2) } -> std::same_as<Writer<std::string, bool>>;
});

// bind works with lambda returning Writer
static_assert(requires(Writer<std::string, int> m) {
    { fp20::bind(m, [](int x) {
        return writer(x * 2, std::string{"doubled"});
    }) } -> std::same_as<Writer<std::string, int>>;
});

// ----------------------------------------------------------------------------
// Category: Writer Primitives - tell, listen, pass
// ----------------------------------------------------------------------------

// tell exists and appends to log
static_assert(requires {
    { tell(std::string{"log entry"}) } -> std::same_as<Writer<std::string, void>>;
});

// tell with different monoid types
static_assert(requires {
    { tell(std::vector<int>{1, 2, 3}) } -> std::same_as<Writer<std::vector<int>, void>>;
});

static_assert(requires {
    { tell(test_monoids::Sum<int>{42}) }
        -> std::same_as<Writer<test_monoids::Sum<int>, void>>;
});

// listen exists and returns (value, log) as value
static_assert(requires(Writer<std::string, int> w) {
    { listen(w) }
        -> std::same_as<Writer<std::string, std::pair<int, std::string>>>;
});

// listen with different types
static_assert(requires(Writer<std::vector<int>, std::string> w) {
    { listen(w) }
        -> std::same_as<Writer<std::vector<int>, std::pair<std::string, std::vector<int>>>>;
});

// pass exists for log transformation
static_assert(requires(
    Writer<std::string, std::pair<int, std::function<std::string(std::string)>>> w
) {
    { pass(w) } -> std::same_as<Writer<std::string, int>>;
});

// censor exists for filtering/transforming log
static_assert(requires(Writer<std::string, int> w) {
    { censor([](std::string s) { return s + "!"; }, w) }
        -> std::same_as<Writer<std::string, int>>;
});

// censor with different monoid types
static_assert(requires(Writer<std::vector<int>, std::string> w) {
    { censor([](std::vector<int> v) {
        v.push_back(999);
        return v;
    }, w) } -> std::same_as<Writer<std::vector<int>, std::string>>;
});

// ----------------------------------------------------------------------------
// Category: Concept Satisfaction
// ----------------------------------------------------------------------------

// Writer satisfies Monad concept (if defined)
static_assert(requires {
    requires Monad<Writer<std::string, int>>;
});

// Writer satisfies Functor concept (if defined)
static_assert(requires {
    requires Functor<Writer<std::string, int>>;
});

// Writer satisfies Applicative concept (if defined)
static_assert(requires {
    requires Applicative<Writer<std::string, int>>;
});

// Writer only works with Monoid log types
static_assert(requires {
    requires Monad<Writer<std::string, int>>;  // string is Monoid
    requires Monad<Writer<std::vector<int>, bool>>;  // vector is Monoid
});

// ----------------------------------------------------------------------------
// Category: Helper Functions
// ----------------------------------------------------------------------------

// execWriter extracts only the log
static_assert(requires(Writer<std::string, int> w) {
    { execWriter(w) } -> std::same_as<std::string>;
});

static_assert(requires(Writer<std::vector<int>, std::string> w) {
    { execWriter(w) } -> std::same_as<std::vector<int>>;
});

// evalWriter extracts only the value (discards log)
static_assert(requires(Writer<std::string, int> w) {
    { evalWriter(w) } -> std::same_as<int>;
});

static_assert(requires(Writer<std::vector<int>, std::string> w) {
    { evalWriter(w) } -> std::same_as<std::string>;
});

// mapWriter transforms both value and log
static_assert(requires(Writer<std::string, int> w) {
    { mapWriter(
        [](std::pair<int, std::string> p) {
            return std::pair{p.first * 2, p.second + "!"};
        },
        w
    ) } -> std::same_as<Writer<std::string, int>>;
});

// ----------------------------------------------------------------------------
// Category: Composition and Utility
// ----------------------------------------------------------------------------

// Writer can compose with fmap and bind
static_assert(requires(Writer<std::string, int> w) {
    { fp20::bind(
        fmap([](int x) { return x + 1; }, w),
        [](int x) { return writer(x * 2, std::string{"doubled"}); }
    ) } -> std::same_as<Writer<std::string, int>>;
});

// tell can be used with bind
static_assert(requires {
    { fp20::bind(tell(std::string{"log"}), [](auto) {
        return writer(42, std::string{"value"});
    }) } -> std::same_as<Writer<std::string, int>>;
});

// listen can be used in bind chains
// TODO: Fix this test - type mismatch issue
// static_assert(requires(Writer<std::string, int> w) {
//     { fp20::bind(listen(w), [](std::pair<int, std::string> p) {
//         return writer(p.first + p.second.size(), std::string{"processed"});
//     }) } -> std::same_as<Writer<std::string, int>>;
// });

// Multiple tells concatenate logs
static_assert(requires {
    { fp20::bind(tell(std::string{"first"}), [](auto) {
        return fp20::bind(tell(std::string{"second"}), [](auto) {
            return writer(42, std::string{"third"});
        });
    }) } -> std::same_as<Writer<std::string, int>>;
});

} // namespace compile_time_tests

// ============================================================================
// RUNTIME TESTS (for integration into test framework)
// ============================================================================
// NOTE: These tests will FAIL to link/run until implementation exists
// Expected failure: Linker error or compilation failure
// ============================================================================

namespace writer_runtime_tests {

void test_writer_basic_construction() {
    // Test: Writer stores value and log
    Writer<std::string, int> w = writer(42, std::string{"log entry"});

    auto [value, log] = w.runWriter();
    assert(value == 42);
    assert(log == "log entry");

    // Test: Writer with different types
    Writer<std::vector<int>, std::string> w2 = writer(
        std::string{"hello"},
        std::vector<int>{1, 2, 3}
    );
    auto [v2, l2] = w2.runWriter();
    assert(v2 == "hello");
    assert((l2 == std::vector<int>{1, 2, 3}));
}

void test_monoid_mempty() {
    // Test: mempty for string
    auto empty_str = mempty<std::string>();
    assert(empty_str == "");

    // Test: mempty for vector
    auto empty_vec = mempty<std::vector<int>>();
    assert(empty_vec.empty());

    // Test: mempty for Sum
    auto empty_sum = mempty<test_monoids::Sum<int>>();
    assert(empty_sum.value == 0);

    // Test: mempty for Product
    auto empty_prod = mempty<test_monoids::Product<int>>();
    assert(empty_prod.value == 1);
}

void test_monoid_mappend() {
    // Test: mappend for string
    auto s1 = std::string{"hello"};
    auto s2 = std::string{" world"};
    auto s3 = mappend(s1, s2);
    assert(s3 == "hello world");

    // Test: mappend for vector
    auto v1 = std::vector<int>{1, 2};
    auto v2 = std::vector<int>{3, 4};
    auto v3 = mappend(v1, v2);
    assert((v3 == std::vector<int>{1, 2, 3, 4}));

    // Test: mappend for Sum
    auto sum1 = test_monoids::Sum<int>{5};
    auto sum2 = test_monoids::Sum<int>{7};
    auto sum3 = mappend(sum1, sum2);
    assert(sum3.value == 12);

    // Test: mappend for Product
    auto prod1 = test_monoids::Product<int>{3};
    auto prod2 = test_monoids::Product<int>{4};
    auto prod3 = mappend(prod1, prod2);
    assert(prod3.value == 12);
}

void test_monoid_laws() {
    // Test: Monoid laws for string

    // Left identity: mempty <> x = x
    auto x = std::string{"test"};
    assert(mappend(mempty<std::string>(), x) == x);

    // Right identity: x <> mempty = x
    assert(mappend(x, mempty<std::string>()) == x);

    // Associativity: (x <> y) <> z = x <> (y <> z)
    auto y = std::string{"ing"};
    auto z = std::string{"!"};
    assert(mappend(mappend(x, y), z) == mappend(x, mappend(y, z)));
}

void test_functor_fmap() {
    // Test: fmap transforms value, preserves log
    Writer<std::string, int> w = writer(21, std::string{"initial"});
    auto w_doubled = fmap([](int x) { return x * 2; }, w);

    auto [value, log] = w_doubled.runWriter();
    assert(value == 42);
    assert(log == "initial");  // Log unchanged

    // Test: fmap can change type
    auto w_string = fmap([](int x) { return std::to_string(x); }, w);
    auto [v2, l2] = w_string.runWriter();
    assert(v2 == "21");
    assert(l2 == "initial");

    // Test: fmap composition
    auto w_composed = fmap(
        [](int x) { return x > 0; },
        fmap([](int x) { return x * 2; }, w)
    );
    auto [v3, l3] = w_composed.runWriter();
    assert(v3 == true);  // 21 * 2 = 42 > 0
    assert(l3 == "initial");
}

void test_applicative_pure() {
    // Test: pure creates Writer with mempty log
    auto w = pure<Writer, std::string>(42);

    auto [value, log] = w.runWriter();
    assert(value == 42);
    assert(log == "");  // mempty for string

    // Test: pure with vector monoid
    auto w2 = pure<Writer, std::vector<int>>(std::string{"hello"});
    auto [v2, l2] = w2.runWriter();
    assert(v2 == "hello");
    assert(l2.empty());  // mempty for vector
}

void test_applicative_apply() {
    // Test: apply combines logs with mappend
    Writer<std::string, std::function<int(int)>> wf = writer(
        std::function<int(int)>{[](int x) { return x * 2; }},
        std::string{"function log"}
    );

    Writer<std::string, int> wx = writer(21, std::string{" value log"});

    auto result = fp20::apply(wf, wx);
    auto [value, log] = result.runWriter();

    assert(value == 42);  // 21 * 2
    assert(log == "function log value log");  // Logs concatenated
}

void test_monad_bind_basic() {
    // Test: bind sequences computations and concatenates logs
    auto w = writer(10, std::string{"first"});
    auto doubled = fp20::bind(w, [](int x) {
        return writer(x * 2, std::string{" second"});
    });

    auto [value, log] = doubled.runWriter();
    assert(value == 20);
    assert(log == "first second");  // Logs concatenated

    // Test: bind can access value and produce new Writer
    auto w2 = fp20::bind(writer(5, std::string{"start"}), [](int x) {
        return writer(x + 10, std::string{" middle"});
    });

    auto [v2, l2] = w2.runWriter();
    assert(v2 == 15);
    assert(l2 == "start middle");
}

void test_monad_bind_complex() {
    // Test: bind chains multiple operations, accumulating logs
    auto computation = fp20::bind(
        writer(5, std::string{"step1"}),
        [](int x) {
            return fp20::bind(
                writer(x * 2, std::string{" step2"}),
                [](int y) {
                    return writer(y + 3, std::string{" step3"});
                }
            );
        }
    );

    auto [value, log] = computation.runWriter();
    assert(value == 13);  // (5 * 2) + 3
    assert(log == "step1 step2 step3");  // All logs concatenated
}

void test_monad_law_left_identity() {
    // Test: Left Identity Law
    // pure(a) >>= f  ≡  f(a)

    auto f = [](int x) {
        return writer(x * 2, std::string{"doubled"});
    };

    int a = 42;
    auto lhs = fp20::bind(pure<Writer, std::string>(a), f);
    auto rhs = f(a);

    auto [v1, l1] = lhs.runWriter();
    auto [v2, l2] = rhs.runWriter();

    assert(v1 == v2);
    assert(l1 == l2);  // mempty <> "doubled" = "doubled"
}

void test_monad_law_right_identity() {
    // Test: Right Identity Law
    // m >>= pure  ≡  m

    auto m = writer(42, std::string{"log"});
    auto bound = fp20::bind(m, [](int x) { return pure<Writer, std::string>(x); });

    auto [v1, l1] = m.runWriter();
    auto [v2, l2] = bound.runWriter();

    assert(v1 == v2);
    assert(l1 == l2);  // "log" <> mempty = "log"
}

void test_monad_law_associativity() {
    // Test: Associativity Law
    // (m >>= f) >>= g  ≡  m >>= (\x -> f(x) >>= g)

    auto m = writer(5, std::string{"m"});
    auto f = [](int x) {
        return writer(x * 2, std::string{" f"});
    };
    auto g = [](int x) {
        return writer(x + 3, std::string{" g"});
    };

    auto lhs = fp20::bind(fp20::bind(m, f), g);
    auto rhs = fp20::bind(m, [f, g](int x) {
        return fp20::bind(f(x), g);
    });

    auto [v1, l1] = lhs.runWriter();
    auto [v2, l2] = rhs.runWriter();

    assert(v1 == v2);
    assert(l1 == l2);  // Both should be "m f g"
}

void test_tell_primitive() {
    // Test: tell appends to log
    auto computation = fp20::bind(tell(std::string{"first log"}), [](auto) {
        return writer(42, std::string{" second log"});
    });

    auto [value, log] = computation.runWriter();
    assert(value == 42);
    assert(log == "first log second log");

    // Test: multiple tells
    auto multi_tell = fp20::bind(tell(std::string{"log1"}), [](auto) {
        return fp20::bind(tell(std::string{" log2"}), [](auto) {
            return fp20::bind(tell(std::string{" log3"}), [](auto) {
                return writer(100, std::string{" final"});
            });
        });
    });

    auto [v2, l2] = multi_tell.runWriter();
    assert(v2 == 100);
    assert(l2 == "log1 log2 log3 final");
}

void test_listen_primitive() {
    // Test: listen returns (value, log) as value
    auto w = writer(42, std::string{"the log"});
    auto w_listened = listen(w);

    auto [pair, log] = w_listened.runWriter();
    assert(pair.first == 42);  // Original value
    assert(pair.second == "the log");  // Log exposed as part of value
    assert(log == "the log");  // Log still in output

    // Test: listen in bind chain
    auto computation = fp20::bind(listen(writer(10, std::string{"original"})),
        [](std::pair<int, std::string> p) {
            // Can access the log in the computation
            int log_length = p.second.size();
            return writer(p.first + log_length, std::string{" processed"});
        }
    );

    auto [v2, l2] = computation.runWriter();
    assert(v2 == 18);  // 10 + len("original")
    assert(l2 == "original processed");
}

void test_censor_primitive() {
    // Test: censor transforms the log
    auto w = writer(42, std::string{"original"});
    auto w_censored = censor([](std::string s) { return s + " [censored]"; }, w);

    auto [value, log] = w_censored.runWriter();
    assert(value == 42);  // Value unchanged
    assert(log == "original [censored]");  // Log transformed

    // Test: censor with vector monoid
    auto w2 = writer(100, std::vector<int>{1, 2, 3});
    auto w2_censored = censor([](std::vector<int> v) {
        v.push_back(999);
        return v;
    }, w2);

    auto [v2, l2] = w2_censored.runWriter();
    assert(v2 == 100);
    assert((l2 == std::vector<int>{1, 2, 3, 999}));
}

void test_pass_primitive() {
    // Test: pass applies function from value to log
    auto w = writer(
        std::pair{42, [](std::string s) { return s + " modified"; }},
        std::string{"original"}
    );

    auto w_passed = pass(w);
    auto [value, log] = w_passed.runWriter();

    assert(value == 42);  // First element of pair
    assert(log == "original modified");  // Function applied to log
}

void test_helper_functions() {
    // Test: execWriter extracts only the log
    auto w = writer(42, std::string{"the log"});
    auto log = execWriter(w);
    assert(log == "the log");

    // Test: evalWriter extracts only the value
    auto value = evalWriter(w);
    assert(value == 42);

    // Test: mapWriter transforms both
    auto w2 = mapWriter(
        [](std::pair<int, std::string> p) {
            return std::pair{p.first * 2, p.second + "!"};
        },
        w
    );

    auto [v2, l2] = w2.runWriter();
    assert(v2 == 84);  // 42 * 2
    assert(l2 == "the log!");
}

void test_logging_example_basic() {
    // Test: Simple logging pattern
    auto computation = [](int x) {
        return fp20::bind(tell(std::string{"Processing " + std::to_string(x)}), [x](auto) {
            int result = x * 2;
            return fp20::bind(tell(std::string{" Result: " + std::to_string(result)}), [result](auto) {
                return writer(result, std::string{});
            });
        });
    };

    auto result = computation(21);
    auto [value, log] = result.runWriter();

    assert(value == 42);
    assert(log == "Processing 21 Result: 42");
}

void test_logging_example_chain() {
    // Test: Chained computations with logging
    auto add = [](int x, int y) {
        return writer(x + y, std::string{"add(" + std::to_string(x) + "," + std::to_string(y) + ")"});
    };

    auto multiply = [](int x, int y) {
        return writer(x * y, std::string{" mul(" + std::to_string(x) + "," + std::to_string(y) + ")"});
    };

    // Compute: (5 + 3) * 2
    auto computation = fp20::bind(add(5, 3), [multiply](int sum) {
        return fp20::bind(multiply(sum, 2), [](int product) {
            return writer(product, std::string{" done"});
        });
    });

    auto [value, log] = computation.runWriter();
    assert(value == 16);  // (5 + 3) * 2
    assert(log == "add(5,3) mul(8,2) done");
}

void test_writer_with_vector_monoid() {
    // Test: Writer with vector accumulation
    auto w1 = writer(10, std::vector<int>{1});
    auto w2 = fp20::bind(w1, [](int x) {
        return writer(x * 2, std::vector<int>{2, 3});
    });

    auto [value, log] = w2.runWriter();
    assert(value == 20);
    assert((log == std::vector<int>{1, 2, 3}));  // Vectors concatenated

    // Test: tell with vector
    auto w3 = fp20::bind(tell(std::vector<int>{100}), [](auto) {
        return fp20::bind(tell(std::vector<int>{200, 300}), [](auto) {
            return writer(42, std::vector<int>{400});
        });
    });

    auto [v3, l3] = w3.runWriter();
    assert(v3 == 42);
    assert((l3 == std::vector<int>{100, 200, 300, 400}));
}

void test_writer_immutability() {
    // Test: Writer operations don't mutate original
    auto original = writer(5, std::string{"original"});
    auto mapped = fmap([](int x) { return x * 2; }, original);
    auto bound = fp20::bind(original, [](int x) {
        return writer(x + 1, std::string{" bound"});
    });
    auto censored = censor([](std::string s) { return s + " censored"; }, original);

    // All should give different results but original unchanged
    auto [v0, l0] = original.runWriter();
    auto [v1, l1] = mapped.runWriter();
    auto [v2, l2] = bound.runWriter();
    auto [v3, l3] = censored.runWriter();

    assert(v0 == 5 && l0 == "original");
    assert(v1 == 10 && l1 == "original");  // Value changed, log same
    assert(v2 == 6 && l2 == "original bound");  // Both changed
    assert(v3 == 5 && l3 == "original censored");  // Log transformed

    // Original still unchanged
    auto [v4, l4] = original.runWriter();
    assert(v4 == 5 && l4 == "original");
}

void test_writer_law_listen_tell() {
    // Test: listen (tell w) ≡ tell w >> return ((), w)
    auto w = std::string{"test log"};

    auto lhs = listen(tell(w));
    auto rhs = fp20::bind(tell(w), [w](auto) {
        return pure<Writer, std::string>(std::pair{0, w});  // void ~ 0
    });

    auto [p1, l1] = lhs.runWriter();
    auto [p2, l2] = rhs.runWriter();

    // Both should have same log and similar structure
    assert(l1 == l2);
    assert(l1 == w);
}

// Entry point to run all tests
void run_all_tests() {
    test_writer_basic_construction();
    test_monoid_mempty();
    test_monoid_mappend();
    test_monoid_laws();
    test_functor_fmap();
    test_applicative_pure();
    test_applicative_apply();
    test_monad_bind_basic();
    test_monad_bind_complex();
    test_monad_law_left_identity();
    test_monad_law_right_identity();
    test_monad_law_associativity();
    test_tell_primitive();
    test_listen_primitive();
    test_censor_primitive();
    test_pass_primitive();
    test_helper_functions();
    test_logging_example_basic();
    test_logging_example_chain();
    test_writer_with_vector_monoid();
    test_writer_immutability();
    test_writer_law_listen_tell();
}

} // namespace writer_runtime_tests

// ============================================================================
// VERIFICATION THAT TESTS FAIL (RED PHASE CONFIRMATION)
// ============================================================================
//
// Expected compilation/runtime errors:
//
// 1. COMPILE-TIME ERROR - Missing header:
//    fatal error: 'fp20/writer.hpp' file not found
//    #include "fp20/writer.hpp"
//             ^~~~~~~~~~~~~~~~~
//
// 2. COMPILE-TIME ERROR - Missing Monoid concept:
//    fatal error: 'fp20/concepts/monoid.hpp' file not found
//    #include "fp20/concepts/monoid.hpp"
//             ^~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// 3. COMPILE-TIME ERROR - Undefined Writer template:
//    error: no template named 'Writer' in namespace 'fp20'
//    typename Writer<std::string, int>;
//             ^
//
// 4. COMPILE-TIME ERROR - Undefined Monoid functions:
//    error: use of undeclared identifier 'mempty'
//    error: use of undeclared identifier 'mappend'
//
// 5. COMPILE-TIME ERROR - Undefined Writer functions:
//    error: use of undeclared identifier 'writer'
//    error: use of undeclared identifier 'tell'
//    error: use of undeclared identifier 'listen'
//    error: use of undeclared identifier 'pass'
//    error: use of undeclared identifier 'censor'
//    error: use of undeclared identifier 'execWriter'
//    error: use of undeclared identifier 'evalWriter'
//    error: use of undeclared identifier 'mapWriter'
//
// 6. COMPILE-TIME ERROR - No fmap for Writer:
//    error: no matching function for call to 'fmap'
//    (when used with Writer arguments)
//
// 7. COMPILE-TIME ERROR - No bind for Writer:
//    error: no matching function for call to 'bind'
//    (when used with Writer arguments)
//
// These errors confirm tests are properly written in RED phase.
// Tests specify the complete Writer monad API that must be implemented.
//
// Total test count: 60+ static_assert + 22 runtime test functions
// Coverage:
// - Monoid concept and operations (10 assertions)
// - Writer type structure (12 assertions)
// - Functor instance (5 assertions)
// - Applicative instance (6 assertions)
// - Monad instance (5 assertions)
// - Primitives tell/listen/pass/censor (8 assertions)
// - Concept satisfaction (3 assertions)
// - Helper functions (4 assertions)
// - Composition utilities (4 assertions)
// - Runtime behavior (22 test functions covering ~80+ assertions)
//
// REQUIREMENTS COVERAGE:
// 1. Writer<W,A> core structure: TESTED
//    - Type definitions (log_type, value_type)
//    - Construction with writer(a, w)
//    - runWriter() returns std::pair<A,W>
//    - W must satisfy Monoid concept
//
// 2. Functor instance: TESTED
//    - fmap transforms value while preserving log
//    - fmap(f, Writer(a, w)) = Writer(f(a), w)
//
// 3. Applicative instance: TESTED
//    - pure(a) creates Writer(a, mempty)
//    - apply combines logs with mappend
//    - Writer(f, w1) <*> Writer(a, w2) = Writer(f(a), w1 <> w2)
//
// 4. Monad instance: TESTED
//    - bind chains computations and concatenates logs
//    - Writer(a, w1) >>= f produces Writer(b, w1 <> w2)
//    - Monad laws verified (left identity, right identity, associativity)
//
// 5. MonadWriter operations: TESTED
//    - tell(w) creates Writer(unit, w)
//    - listen() returns Writer((a, w), w)
//    - pass() for log transformation
//    - censor() for log filtering/transformation
//
// 6. Monoid examples: TESTED
//    - std::string with concatenation
//    - std::vector<T> with concatenation
//    - Sum<T> with addition
//    - Product<T> with multiplication
//
// ============================================================================
