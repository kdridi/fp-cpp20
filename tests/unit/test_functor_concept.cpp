// TDD: RED PHASE - Writing failing test FIRST
// Test for Functor concept definition
// This test MUST fail initially, proving we're following TDD

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

// This include will fail initially - that's the TDD way!
// We're testing that our Functor concept exists and works correctly
#include <fp20/concepts/functor.hpp>

#include <vector>
#include <optional>
#include <string>

// Test fixture for compile-time concept checking
template<typename T>
struct is_functor_testable {
    static constexpr bool value = false;
};

// Once implemented, this should be true for valid functors
template<typename T>
    requires fp20::Functor<T>
struct is_functor_testable<T> {
    static constexpr bool value = true;
};

TEST_CASE("Functor concept basics", "[functor][concepts][tdd]") {
    SECTION("Functor concept exists and is detectable") {
        // These assertions will fail until we implement the Functor concept
        STATIC_REQUIRE(is_functor_testable<std::vector<int>>::value);
        STATIC_REQUIRE(is_functor_testable<std::optional<int>>::value);
    }

    SECTION("Non-functors are correctly rejected") {
        // These should NOT satisfy the Functor concept
        STATIC_REQUIRE_FALSE(is_functor_testable<int>::value);
        STATIC_REQUIRE_FALSE(is_functor_testable<std::string>::value);
    }
}

TEST_CASE("Functor fmap exists", "[functor][fmap][tdd]") {
    SECTION("fmap can be called on std::vector") {
        std::vector<int> vec{1, 2, 3};
        auto add_one = [](int x) { return x + 1; };

        // This will fail to compile initially - TDD!
        auto result = fp20::fmap(add_one, vec);

        REQUIRE(result == std::vector<int>{2, 3, 4});
    }

    SECTION("fmap can be called on std::optional") {
        std::optional<int> some_value = 42;
        auto double_it = [](int x) { return x * 2; };

        // This will fail to compile initially - TDD!
        auto result = fp20::fmap(double_it, some_value);

        REQUIRE(result.has_value());
        REQUIRE(result.value() == 84);
    }

    SECTION("fmap preserves emptiness for optional") {
        std::optional<int> no_value = std::nullopt;
        auto double_it = [](int x) { return x * 2; };

        // This will fail to compile initially - TDD!
        auto result = fp20::fmap(double_it, no_value);

        REQUIRE_FALSE(result.has_value());
    }
}

TEST_CASE("Functor supports different function signatures", "[functor][fmap][types][tdd]") {
    SECTION("fmap with type transformation") {
        std::vector<int> numbers{1, 2, 3};
        auto to_string = [](int x) { return std::to_string(x); };

        // This will fail to compile initially - TDD!
        auto result = fp20::fmap(to_string, numbers);

        REQUIRE(result == std::vector<std::string>{"1", "2", "3"});
    }

    SECTION("fmap with member function pointers") {
        std::vector<std::string> strings{"hello", "world"};

        // This will fail to compile initially - TDD!
        auto result = fp20::fmap(&std::string::size, strings);

        REQUIRE(result == std::vector<size_t>{5, 5});
    }
}