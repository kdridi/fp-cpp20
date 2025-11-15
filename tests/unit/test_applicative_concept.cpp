// TDD: RED PHASE - Writing failing test FIRST
// Test for Applicative concept behavioral implementation
// This test MUST fail initially, proving we're following TDD

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

// This include will fail initially - that's the TDD way!
#include <fp20/concepts/applicative.hpp>

#include <vector>
#include <optional>
#include <string>
#include <functional>

TEST_CASE("Applicative pure function", "[applicative][pure][tdd]") {
    SECTION("pure wraps value into std::vector") {
        auto result = fp20::pure<std::vector>(42);

        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 42);
    }

    SECTION("pure wraps value into std::optional") {
        auto result = fp20::pure<std::optional>(42);

        REQUIRE(result.has_value());
        REQUIRE(result.value() == 42);
    }

    SECTION("pure works with different types") {
        auto str_result = fp20::pure<std::vector>(std::string("hello"));

        REQUIRE(str_result.size() == 1);
        REQUIRE(str_result[0] == "hello");
    }

    SECTION("pure can be chained with fmap") {
        auto result = fp20::fmap([](int x) { return x * 2; },
                                 fp20::pure<std::optional>(21));

        REQUIRE(result.has_value());
        REQUIRE(result.value() == 42);
    }
}

TEST_CASE("Applicative apply function with std::optional", "[applicative][apply][optional][tdd]") {
    SECTION("apply with Some function and Some value") {
        auto func = std::optional{std::function<int(int)>([](int x) { return x * 2; })};
        auto value = std::optional{21};

        auto result = fp20::apply(func, value);

        REQUIRE(result.has_value());
        REQUIRE(result.value() == 42);
    }

    SECTION("apply with None function returns None") {
        std::optional<std::function<int(int)>> func = std::nullopt;
        auto value = std::optional{42};

        auto result = fp20::apply(func, value);

        REQUIRE_FALSE(result.has_value());
    }

    SECTION("apply with Some function and None value returns None") {
        auto func = std::optional{std::function<int(int)>([](int x) { return x * 2; })};
        std::optional<int> value = std::nullopt;

        auto result = fp20::apply(func, value);

        REQUIRE_FALSE(result.has_value());
    }

    SECTION("apply with None function and None value returns None") {
        std::optional<std::function<int(int)>> func = std::nullopt;
        std::optional<int> value = std::nullopt;

        auto result = fp20::apply(func, value);

        REQUIRE_FALSE(result.has_value());
    }

    SECTION("apply supports type transformation") {
        auto func = std::optional{std::function<std::string(int)>([](int x) {
            return std::to_string(x);
        })};
        auto value = std::optional{42};

        auto result = fp20::apply(func, value);

        REQUIRE(result.has_value());
        REQUIRE(result.value() == "42");
    }
}

TEST_CASE("Applicative apply function with std::vector", "[applicative][apply][vector][tdd]") {
    SECTION("apply with single function and single value") {
        std::vector<std::function<int(int)>> funcs{[](int x) { return x * 2; }};
        std::vector<int> values{21};

        auto result = fp20::apply(funcs, values);

        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 42);
    }

    SECTION("apply with single function and multiple values") {
        std::vector<std::function<int(int)>> funcs{[](int x) { return x * 2; }};
        std::vector<int> values{1, 2, 3};

        auto result = fp20::apply(funcs, values);

        REQUIRE(result == std::vector<int>{2, 4, 6});
    }

    SECTION("apply with multiple functions and single value") {
        std::vector<std::function<int(int)>> funcs{
            [](int x) { return x * 2; },
            [](int x) { return x + 10; }
        };
        std::vector<int> values{5};

        auto result = fp20::apply(funcs, values);

        REQUIRE(result == std::vector<int>{10, 15});
    }

    SECTION("apply with multiple functions and multiple values (cartesian product)") {
        std::vector<std::function<int(int)>> funcs{
            [](int x) { return x * 2; },
            [](int x) { return x + 10; }
        };
        std::vector<int> values{1, 2};

        auto result = fp20::apply(funcs, values);

        // Cartesian product: [f1(v1), f1(v2), f2(v1), f2(v2)]
        REQUIRE(result == std::vector<int>{2, 4, 11, 12});
    }

    SECTION("apply with empty function vector returns empty") {
        std::vector<std::function<int(int)>> funcs{};
        std::vector<int> values{1, 2, 3};

        auto result = fp20::apply(funcs, values);

        REQUIRE(result.empty());
    }

    SECTION("apply with empty value vector returns empty") {
        std::vector<std::function<int(int)>> funcs{[](int x) { return x * 2; }};
        std::vector<int> values{};

        auto result = fp20::apply(funcs, values);

        REQUIRE(result.empty());
    }

    SECTION("apply supports type transformation with vectors") {
        std::vector<std::function<std::string(int)>> funcs{
            [](int x) { return std::to_string(x); }
        };
        std::vector<int> values{1, 2, 3};

        auto result = fp20::apply(funcs, values);

        REQUIRE(result == std::vector<std::string>{"1", "2", "3"});
    }
}

TEST_CASE("Applicative multi-parameter lifting pattern", "[applicative][lifting][tdd]") {
    SECTION("lift binary function with std::optional") {
        auto add = [](int x) {
            return [x](int y) { return x + y; };
        };

        auto x = std::optional{10};
        auto y = std::optional{32};

        // Lifting: pure add <*> x <*> y
        auto result = fp20::apply(
            fp20::fmap(add, x),
            y
        );

        REQUIRE(result.has_value());
        REQUIRE(result.value() == 42);
    }

    SECTION("lift binary function with std::vector") {
        auto multiply = [](int x) {
            return [x](int y) { return x * y; };
        };

        std::vector<int> xs{1, 2};
        std::vector<int> ys{3, 4};

        // Lifting: fmap multiply xs <*> ys
        auto result = fp20::apply(
            fp20::fmap(multiply, xs),
            ys
        );

        // Result: [1*3, 1*4, 2*3, 2*4] = [3, 4, 6, 8]
        REQUIRE(result == std::vector<int>{3, 4, 6, 8});
    }
}
