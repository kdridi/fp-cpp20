// TDD: RED PHASE - Testing Functor laws
// These tests verify that our Functor implementation follows the mathematical laws:
// 1. Identity: fmap id = id
// 2. Composition: fmap (f . g) = fmap f . fmap g

#include <catch2/catch_test_macros.hpp>
#include <fp20/concepts/functor.hpp>

#include <vector>
#include <optional>
#include <functional>

// Identity function
auto identity = [](auto x) { return x; };

// Helper to compose functions
template<typename F, typename G>
auto compose(F f, G g) {
    return [f, g](auto x) { return f(g(x)); };
}

TEST_CASE("Functor Identity Law", "[functor][laws][tdd]") {
    SECTION("Vector satisfies identity law") {
        std::vector<int> vec{1, 2, 3, 4, 5};

        // fmap id vec == vec
        auto result = fp20::fmap(identity, vec);
        REQUIRE(result == vec);
    }

    SECTION("Optional satisfies identity law") {
        std::optional<int> some = 42;
        std::optional<int> none = std::nullopt;

        // fmap id some == some
        auto result_some = fp20::fmap(identity, some);
        REQUIRE(result_some == some);

        // fmap id none == none
        auto result_none = fp20::fmap(identity, none);
        REQUIRE(result_none == none);
    }
}

TEST_CASE("Functor Composition Law", "[functor][laws][tdd]") {
    auto add_one = [](int x) { return x + 1; };
    auto multiply_two = [](int x) { return x * 2; };
    auto composed = compose(multiply_two, add_one);

    SECTION("Vector satisfies composition law") {
        std::vector<int> vec{1, 2, 3};

        // fmap (f . g) vec == (fmap f . fmap g) vec
        auto left = fp20::fmap(composed, vec);
        auto right = fp20::fmap(multiply_two, fp20::fmap(add_one, vec));

        REQUIRE(left == right);
        REQUIRE(left == std::vector<int>{4, 6, 8});
    }

    SECTION("Optional satisfies composition law") {
        std::optional<int> some = 10;

        // fmap (f . g) some == (fmap f . fmap g) some
        auto left = fp20::fmap(composed, some);
        auto right = fp20::fmap(multiply_two, fp20::fmap(add_one, some));

        REQUIRE(left == right);
        REQUIRE(left.value() == 22);
    }

    SECTION("Composition law holds for empty optional") {
        std::optional<int> none = std::nullopt;

        auto left = fp20::fmap(composed, none);
        auto right = fp20::fmap(multiply_two, fp20::fmap(add_one, none));

        REQUIRE(left == right);
        REQUIRE_FALSE(left.has_value());
    }
}

TEST_CASE("Functor laws with different types", "[functor][laws][types][tdd]") {
    auto to_string = [](int x) { return std::to_string(x); };
    auto append_exclaim = [](std::string s) { return s + "!"; };
    auto int_to_exciting_string = compose(append_exclaim, to_string);

    SECTION("Composition law with type transformation") {
        std::vector<int> vec{1, 2, 3};

        auto left = fp20::fmap(int_to_exciting_string, vec);
        auto right = fp20::fmap(append_exclaim, fp20::fmap(to_string, vec));

        REQUIRE(left == right);
        REQUIRE(left == std::vector<std::string>{"1!", "2!", "3!"});
    }
}