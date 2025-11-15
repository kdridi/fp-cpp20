// ============================================
// RUNTIME TESTS FOR MONAD OPERATIONS
// ============================================
// TDD RED PHASE: These tests will FAIL initially
// Expected errors: Linker errors for undefined return_ and bind functions
//                  Assertion failures once functions exist but are incorrect
//
// Following TDD: Tests written BEFORE implementation
// Story: FP-004a (Monad Core Concept Definition)
//
// These tests complement the compile-time tests with runtime behavior verification.
// They should be included in main/main.cpp as assertion-based tests.

#include <cassert>
#include <vector>
#include <optional>
#include <string>
#include <functional>
#include <iostream>

// Will fail to link - functions not implemented yet
#include <fp20/concepts/monad.hpp>
#include <fp20/concepts/applicative.hpp>
#include <fp20/concepts/functor.hpp>

// ============================================
// RUNTIME TEST UTILITIES
// ============================================

// Helper to print test progress (optional, for debugging)
inline void test_section(const char* name) {
    std::cout << "\n=== Testing: " << name << " ===\n";
}

// ============================================
// TEST SUITE 1: return_ RUNTIME BEHAVIOR
// ============================================
// Verify that return_ correctly lifts values into monadic context

inline void test_return_optional() {
    test_section("return_ with std::optional");

    // Test 1.1: return_ creates Some with the given value
    // EXPECTED FAILURE: Linker error - fp20::return_ undefined
    auto opt_int = fp20::return_<std::optional>(42);
    assert(opt_int.has_value());
    assert(opt_int.value() == 42);

    // Test 1.2: return_ with string
    auto opt_str = fp20::return_<std::optional>(std::string("hello"));
    assert(opt_str.has_value());
    assert(opt_str.value() == "hello");

    // Test 1.3: return_ with double
    auto opt_double = fp20::return_<std::optional>(3.14);
    assert(opt_double.has_value());
    assert(opt_double.value() == 3.14);

    // Test 1.4: return_ with complex object
    struct Point { int x, y; };
    auto opt_point = fp20::return_<std::optional>(Point{10, 20});
    assert(opt_point.has_value());
    assert(opt_point.value().x == 10);
    assert(opt_point.value().y == 20);

    // Test 1.5: return_ should create minimal context (not nested)
    auto opt = fp20::return_<std::optional>(5);
    static_assert(std::is_same_v<decltype(opt), std::optional<int>>);
    // Should NOT be std::optional<std::optional<int>>
}

inline void test_return_vector() {
    test_section("return_ with std::vector");

    // Test 2.1: return_ creates singleton vector
    // EXPECTED FAILURE: Linker error - fp20::return_ undefined
    auto vec_int = fp20::return_<std::vector>(42);
    assert(vec_int.size() == 1);
    assert(vec_int[0] == 42);

    // Test 2.2: return_ with string
    auto vec_str = fp20::return_<std::vector>(std::string("hello"));
    assert(vec_str.size() == 1);
    assert(vec_str[0] == "hello");

    // Test 2.3: return_ with double
    auto vec_double = fp20::return_<std::vector>(2.718);
    assert(vec_double.size() == 1);
    assert(vec_double[0] == 2.718);

    // Test 2.4: Verify singleton property (exactly one element)
    auto vec = fp20::return_<std::vector>(100);
    assert(vec.size() == 1);
    assert(vec.front() == 100);
    assert(vec.back() == 100);

    // Test 2.5: return_ should be equivalent to pure for vectors
    auto ret_vec = fp20::return_<std::vector>(42);
    auto pure_vec = fp20::pure<std::vector>(42);
    assert(ret_vec.size() == pure_vec.size());
    assert(ret_vec[0] == pure_vec[0]);
}

// ============================================
// TEST SUITE 2: bind RUNTIME BEHAVIOR FOR OPTIONAL
// ============================================
// Verify that bind correctly sequences optional computations

inline void test_bind_optional_basic() {
    test_section("bind with std::optional - basic operations");

    // Test 3.1: bind with Some and function returning Some
    // EXPECTED FAILURE: Linker error - fp20::bind undefined
    auto result1 = fp20::bind(
        std::optional<int>{21},
        [](int x) { return std::optional<int>{x * 2}; }
    );
    assert(result1.has_value());
    assert(result1.value() == 42);

    // Test 3.2: bind with None propagates None
    auto result2 = fp20::bind(
        std::optional<int>{},  // None
        [](int x) { return std::optional<int>{x * 2}; }
    );
    assert(!result2.has_value());

    // Test 3.3: bind where function returns None
    auto result3 = fp20::bind(
        std::optional<int>{100},
        [](int x) -> std::optional<int> {
            return x > 50 ? std::nullopt : std::optional<int>{x};
        }
    );
    assert(!result3.has_value());

    // Test 3.4: bind where function returns Some based on condition
    auto result4 = fp20::bind(
        std::optional<int>{25},
        [](int x) -> std::optional<int> {
            return x > 50 ? std::nullopt : std::optional<int>{x * 2};
        }
    );
    assert(result4.has_value());
    assert(result4.value() == 50);
}

inline void test_bind_optional_type_transform() {
    test_section("bind with std::optional - type transformations");

    // Test 4.1: Transform int to string
    auto result1 = fp20::bind(
        std::optional<int>{42},
        [](int x) { return std::optional<std::string>{std::to_string(x)}; }
    );
    assert(result1.has_value());
    assert(result1.value() == "42");

    // Test 4.2: Transform string to int (length)
    auto result2 = fp20::bind(
        std::optional<std::string>{"hello"},
        [](const std::string& s) {
            return std::optional<int>{static_cast<int>(s.length())};
        }
    );
    assert(result2.has_value());
    assert(result2.value() == 5);

    // Test 4.3: Transform int to double
    auto result3 = fp20::bind(
        std::optional<int>{10},
        [](int x) { return std::optional<double>{x * 1.5}; }
    );
    assert(result3.has_value());
    assert(result3.value() == 15.0);

    // Test 4.4: Chain transformations with multiple binds
    auto result4 = fp20::bind(
        fp20::bind(
            std::optional<int>{21},
            [](int x) { return std::optional<int>{x * 2}; }
        ),
        [](int x) { return std::optional<std::string>{std::to_string(x)}; }
    );
    assert(result4.has_value());
    assert(result4.value() == "42");
}

inline void test_bind_optional_chaining() {
    test_section("bind with std::optional - monadic chaining");

    // Test 5.1: Safe division using bind
    auto safe_div = [](int a, int b) -> std::optional<int> {
        return b != 0 ? std::optional<int>{a / b} : std::nullopt;
    };

    auto result1 = fp20::bind(
        std::optional<int>{100},
        [&](int x) { return safe_div(x, 2); }
    );
    assert(result1.has_value());
    assert(result1.value() == 50);

    auto result2 = fp20::bind(
        std::optional<int>{100},
        [&](int x) { return safe_div(x, 0); }  // Division by zero
    );
    assert(!result2.has_value());

    // Test 5.2: Multiple chained safe operations
    auto result3 = fp20::bind(
        fp20::bind(
            std::optional<int>{100},
            [&](int x) { return safe_div(x, 2); }  // 50
        ),
        [&](int x) { return safe_div(x, 5); }  // 10
    );
    assert(result3.has_value());
    assert(result3.value() == 10);

    // Test 5.3: Chain breaks on first None
    auto result4 = fp20::bind(
        fp20::bind(
            std::optional<int>{100},
            [&](int x) { return safe_div(x, 0); }  // None
        ),
        [&](int x) { return safe_div(x, 5); }  // Should not execute
    );
    assert(!result4.has_value());
}

// ============================================
// TEST SUITE 3: bind RUNTIME BEHAVIOR FOR VECTOR
// ============================================
// Verify that bind correctly implements list monad behavior

inline void test_bind_vector_basic() {
    test_section("bind with std::vector - basic operations");

    // Test 6.1: bind with simple replication
    // EXPECTED FAILURE: Linker error - fp20::bind undefined
    auto result1 = fp20::bind(
        std::vector<int>{1, 2, 3},
        [](int x) { return std::vector<int>{x, x}; }  // Duplicate each
    );
    assert(result1.size() == 6);
    assert(result1[0] == 1 && result1[1] == 1);
    assert(result1[2] == 2 && result1[3] == 2);
    assert(result1[4] == 3 && result1[5] == 3);

    // Test 6.2: bind with transformation
    auto result2 = fp20::bind(
        std::vector<int>{1, 2, 3},
        [](int x) { return std::vector<int>{x * 10}; }  // Singleton transform
    );
    assert(result2.size() == 3);
    assert(result2[0] == 10);
    assert(result2[1] == 20);
    assert(result2[2] == 30);

    // Test 6.3: bind with empty result
    auto result3 = fp20::bind(
        std::vector<int>{1, 2, 3},
        [](int x) { return std::vector<int>{}; }  // Always empty
    );
    assert(result3.empty());

    // Test 6.4: bind with empty input
    auto result4 = fp20::bind(
        std::vector<int>{},
        [](int x) { return std::vector<int>{x, x}; }
    );
    assert(result4.empty());
}

inline void test_bind_vector_flatmap() {
    test_section("bind with std::vector - flatMap behavior");

    // Test 7.1: Flatten nested structure
    auto result1 = fp20::bind(
        std::vector<int>{1, 2, 3},
        [](int x) {
            std::vector<int> v;
            for (int i = 0; i < x; ++i) {
                v.push_back(x);
            }
            return v;
        }
    );
    // 1 appears 1 time, 2 appears 2 times, 3 appears 3 times
    assert(result1.size() == 6);  // 1 + 2 + 3
    assert(result1[0] == 1);
    assert(result1[1] == 2 && result1[2] == 2);
    assert(result1[3] == 3 && result1[4] == 3 && result1[5] == 3);

    // Test 7.2: Type transformation with flattening
    auto result2 = fp20::bind(
        std::vector<int>{1, 2, 3},
        [](int x) {
            return std::vector<std::string>{std::to_string(x), std::to_string(x * 10)};
        }
    );
    assert(result2.size() == 6);
    assert(result2[0] == "1" && result2[1] == "10");
    assert(result2[2] == "2" && result2[3] == "20");
    assert(result2[4] == "3" && result2[5] == "30");

    // Test 7.3: Conditional expansion
    auto result3 = fp20::bind(
        std::vector<int>{1, 2, 3, 4, 5},
        [](int x) {
            // Only keep even numbers, duplicate them
            return x % 2 == 0 ? std::vector<int>{x, x} : std::vector<int>{};
        }
    );
    assert(result3.size() == 4);  // 2, 2, 4, 4
    assert(result3[0] == 2 && result3[1] == 2);
    assert(result3[2] == 4 && result3[3] == 4);
}

inline void test_bind_vector_chaining() {
    test_section("bind with std::vector - monadic chaining");

    // Test 8.1: Chain multiple binds
    auto result1 = fp20::bind(
        fp20::bind(
            std::vector<int>{1, 2},
            [](int x) { return std::vector<int>{x, x + 10}; }  // {1,11,2,12}
        ),
        [](int x) { return std::vector<int>{x * 2}; }  // {2,22,4,24}
    );
    assert(result1.size() == 4);
    assert(result1[0] == 2 && result1[1] == 22);
    assert(result1[2] == 4 && result1[3] == 24);

    // Test 8.2: Cartesian product via chaining
    auto result2 = fp20::bind(
        std::vector<int>{1, 2},
        [](int x) {
            return fp20::bind(
                std::vector<int>{10, 20},
                [x](int y) { return std::vector<int>{x + y}; }
            );
        }
    );
    assert(result2.size() == 4);  // 2 * 2
    assert(result2[0] == 11);  // 1 + 10
    assert(result2[1] == 21);  // 1 + 20
    assert(result2[2] == 12);  // 2 + 10
    assert(result2[3] == 22);  // 2 + 20
}

// ============================================
// TEST SUITE 4: MONAD LAWS (Informal Runtime Tests)
// ============================================
// These are simplified runtime checks of monad laws
// Full property-based testing will be in FP-004b

inline void test_monad_law_left_identity() {
    test_section("Monad Law: Left Identity (return a >>= f ≡ f a)");

    // Test 9.1: Left identity for optional
    auto f = [](int x) { return std::optional<int>{x * 2}; };
    auto value = 21;

    auto left = fp20::bind(fp20::return_<std::optional>(value), f);
    auto right = f(value);

    assert(left.has_value() == right.has_value());
    if (left.has_value()) {
        assert(left.value() == right.value());
    }

    // Test 9.2: Left identity for vector
    auto g = [](int x) { return std::vector<int>{x, x + 1}; };
    auto value2 = 5;

    auto left_vec = fp20::bind(fp20::return_<std::vector>(value2), g);
    auto right_vec = g(value2);

    assert(left_vec.size() == right_vec.size());
    for (size_t i = 0; i < left_vec.size(); ++i) {
        assert(left_vec[i] == right_vec[i]);
    }
}

inline void test_monad_law_right_identity() {
    test_section("Monad Law: Right Identity (m >>= return ≡ m)");

    // Test 10.1: Right identity for optional
    auto m1 = std::optional<int>{42};
    auto result1 = fp20::bind(m1, [](int x) { return fp20::return_<std::optional>(x); });

    assert(result1.has_value() == m1.has_value());
    if (result1.has_value()) {
        assert(result1.value() == m1.value());
    }

    // Test 10.2: Right identity for None
    auto m2 = std::optional<int>{};
    auto result2 = fp20::bind(m2, [](int x) { return fp20::return_<std::optional>(x); });
    assert(!result2.has_value());

    // Test 10.3: Right identity for vector
    auto m3 = std::vector<int>{1, 2, 3};
    auto result3 = fp20::bind(m3, [](int x) { return fp20::return_<std::vector>(x); });

    assert(result3.size() == m3.size());
    for (size_t i = 0; i < m3.size(); ++i) {
        assert(result3[i] == m3[i]);
    }
}

inline void test_monad_law_associativity() {
    test_section("Monad Law: Associativity ((m >>= f) >>= g ≡ m >>= (λx -> f x >>= g))");

    // Test 11.1: Associativity for optional
    auto m = std::optional<int>{10};
    auto f = [](int x) { return std::optional<int>{x + 5}; };
    auto g = [](int x) { return std::optional<int>{x * 2}; };

    // Left: (m >>= f) >>= g
    auto left = fp20::bind(fp20::bind(m, f), g);

    // Right: m >>= (λx -> f x >>= g)
    auto right = fp20::bind(m, [f, g](int x) { return fp20::bind(f(x), g); });

    assert(left.has_value() == right.has_value());
    if (left.has_value()) {
        assert(left.value() == right.value());
    }

    // Test 11.2: Associativity for vector
    auto m_vec = std::vector<int>{1, 2};
    auto f_vec = [](int x) { return std::vector<int>{x, x + 1}; };
    auto g_vec = [](int x) { return std::vector<int>{x * 10}; };

    auto left_vec = fp20::bind(fp20::bind(m_vec, f_vec), g_vec);
    auto right_vec = fp20::bind(m_vec, [f_vec, g_vec](int x) {
        return fp20::bind(f_vec(x), g_vec);
    });

    assert(left_vec.size() == right_vec.size());
    for (size_t i = 0; i < left_vec.size(); ++i) {
        assert(left_vec[i] == right_vec[i]);
    }
}

// ============================================
// TEST SUITE 5: INTEGRATION WITH APPLICATIVE
// ============================================
// Verify that Monad operations work alongside Applicative operations

inline void test_monad_applicative_integration() {
    test_section("Monad and Applicative integration");

    // Test 12.1: fmap still works on monads
    auto opt = std::optional<int>{21};
    auto fmapped = fp20::fmap([](int x) { return x * 2; }, opt);
    assert(fmapped.has_value());
    assert(fmapped.value() == 42);

    // Test 12.2: pure creates same result as return_
    auto pure_opt = fp20::pure<std::optional>(42);
    auto return_opt = fp20::return_<std::optional>(42);
    assert(pure_opt.has_value() && return_opt.has_value());
    assert(pure_opt.value() == return_opt.value());

    // Test 12.3: apply still works on monads
    auto opt_func = std::optional<std::function<int(int)>>{[](int x) { return x + 10; }};
    auto opt_val = std::optional<int>{5};
    auto applied = fp20::apply(opt_func, opt_val);
    assert(applied.has_value());
    assert(applied.value() == 15);

    // Test 12.4: Can mix bind and fmap
    auto result = fp20::bind(
        fp20::fmap([](int x) { return x * 2; }, std::optional<int>{10}),
        [](int x) { return std::optional<int>{x + 5}; }
    );
    assert(result.has_value());
    assert(result.value() == 25);  // (10 * 2) + 5
}

// ============================================
// MAIN TEST RUNNER
// ============================================
// This function should be called from main/main.cpp

inline void run_all_monad_runtime_tests() {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "MONAD RUNTIME TESTS - TDD RED PHASE\n";
    std::cout << std::string(60, '=') << "\n";

    // Suite 1: return_ tests
    test_return_optional();
    test_return_vector();

    // Suite 2: bind with optional
    test_bind_optional_basic();
    test_bind_optional_type_transform();
    test_bind_optional_chaining();

    // Suite 3: bind with vector
    test_bind_vector_basic();
    test_bind_vector_flatmap();
    test_bind_vector_chaining();

    // Suite 4: Monad laws
    test_monad_law_left_identity();
    test_monad_law_right_identity();
    test_monad_law_associativity();

    // Suite 5: Integration tests
    test_monad_applicative_integration();

    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "ALL MONAD RUNTIME TESTS PASSED!\n";
    std::cout << std::string(60, '=') << "\n\n";
}

// ============================================
// EXPECTED RUNTIME ERRORS (TDD RED PHASE)
// ============================================
/*
 * When this code is linked and run, we expect these errors:
 *
 * PHASE 1: Linker Errors
 * ----------------------
 * ERROR: Undefined reference to 'fp20::return_<std::optional>'
 *    Location: test_return_optional(), line ~58
 *    Meaning: return_ function not implemented yet
 *    Next Step: Implement return_ for std::optional
 *
 * ERROR: Undefined reference to 'fp20::return_<std::vector>'
 *    Location: test_return_vector(), line ~88
 *    Meaning: return_ function not implemented yet
 *    Next Step: Implement return_ for std::vector
 *
 * ERROR: Undefined reference to 'fp20::bind'
 *    Location: test_bind_optional_basic(), line ~123
 *    Meaning: bind function not implemented yet
 *    Next Step: Implement bind for std::optional and std::vector
 *
 * PHASE 2: Runtime Assertion Failures (once functions exist)
 * ----------------------------------------------------------
 * If implementations are incorrect, assertions will fail at runtime.
 * Each assertion verifies specific behavior of monadic operations.
 *
 * These tests guide the implementation to ensure correct behavior.
 */
