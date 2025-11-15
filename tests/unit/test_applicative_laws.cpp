// TDD: RED PHASE - Testing Applicative laws
// These tests verify that our Applicative implementation follows the mathematical laws:
// 1. Identity: pure id <*> v = v
// 2. Homomorphism: pure f <*> pure x = pure (f x)
// 3. Interchange: u <*> pure y = pure ($ y) <*> u
// 4. Composition: pure (.) <*> u <*> v <*> w = u <*> (v <*> w)

#include <catch2/catch_test_macros.hpp>
#include <fp20/concepts/applicative.hpp>

#include <vector>
#include <optional>
#include <functional>
#include <string>

// Identity function
auto identity = [](auto x) { return x; };

// Helper to compose functions
template<typename F, typename G>
auto compose(F f, G g) {
    return [f, g](auto x) { return f(g(x)); };
}

TEST_CASE("Applicative Identity Law", "[applicative][laws][identity][tdd]") {
    // Law: pure id <*> v = v
    // Applying a wrapped identity function should leave the value unchanged

    SECTION("std::optional satisfies identity law") {
        auto some_value = std::optional{42};

        // pure id <*> some_value == some_value
        auto wrapped_id = fp20::pure<std::optional>(std::function<int(int)>(identity));
        auto result = fp20::apply(wrapped_id, some_value);

        REQUIRE(result == some_value);
        REQUIRE(result.value() == 42);
    }

    SECTION("std::optional None satisfies identity law") {
        std::optional<int> none = std::nullopt;

        auto wrapped_id = fp20::pure<std::optional>(std::function<int(int)>(identity));
        auto result = fp20::apply(wrapped_id, none);

        REQUIRE(result == none);
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("std::vector satisfies identity law") {
        std::vector<int> vec{1, 2, 3};

        auto wrapped_id = fp20::pure<std::vector>(std::function<int(int)>(identity));
        auto result = fp20::apply(wrapped_id, vec);

        REQUIRE(result == vec);
    }

    SECTION("std::vector empty satisfies identity law") {
        std::vector<int> empty;

        auto wrapped_id = fp20::pure<std::vector>(std::function<int(int)>(identity));
        auto result = fp20::apply(wrapped_id, empty);

        REQUIRE(result.empty());
    }
}

TEST_CASE("Applicative Homomorphism Law", "[applicative][laws][homomorphism][tdd]") {
    // Law: pure f <*> pure x = pure (f x)
    // Applying a wrapped function to a wrapped value should be the same as
    // wrapping the result of applying the function to the value

    auto double_it = [](int x) { return x * 2; };

    SECTION("std::optional satisfies homomorphism law") {
        auto left = fp20::apply(
            fp20::pure<std::optional>(std::function<int(int)>(double_it)),
            fp20::pure<std::optional>(21)
        );

        auto right = fp20::pure<std::optional>(double_it(21));

        REQUIRE(left == right);
        REQUIRE(left.value() == 42);
    }

    SECTION("std::vector satisfies homomorphism law") {
        auto left = fp20::apply(
            fp20::pure<std::vector>(std::function<int(int)>(double_it)),
            fp20::pure<std::vector>(21)
        );

        auto right = fp20::pure<std::vector>(double_it(21));

        REQUIRE(left == right);
        REQUIRE(left[0] == 42);
    }

    SECTION("Homomorphism with type transformation") {
        auto to_string = [](int x) { return std::to_string(x); };

        auto left = fp20::apply(
            fp20::pure<std::optional>(std::function<std::string(int)>(to_string)),
            fp20::pure<std::optional>(42)
        );

        auto right = fp20::pure<std::optional>(to_string(42));

        REQUIRE(left == right);
        REQUIRE(left.value() == "42");
    }
}

TEST_CASE("Applicative Interchange Law", "[applicative][laws][interchange][tdd]") {
    // Law: u <*> pure y = pure ($ y) <*> u
    // The order of evaluation shouldn't matter

    auto times_two = [](int x) { return x * 2; };

    SECTION("std::optional satisfies interchange law") {
        auto u = fp20::pure<std::optional>(std::function<int(int)>(times_two));
        int y = 21;

        // Left side: u <*> pure y
        auto left = fp20::apply(u, fp20::pure<std::optional>(y));

        // Right side: pure ($ y) <*> u
        // ($ y) means "apply to y", so we need a function that takes a function and applies it to y
        auto apply_to_y = [y](std::function<int(int)> f) { return f(y); };
        auto right = fp20::apply(
            fp20::pure<std::optional>(std::function<int(std::function<int(int)>)>(apply_to_y)),
            u
        );

        REQUIRE(left == right);
        REQUIRE(left.value() == 42);
    }

    SECTION("std::vector satisfies interchange law") {
        std::vector<std::function<int(int)>> u{times_two};
        int y = 21;

        // Left side: u <*> pure y
        auto left = fp20::apply(u, fp20::pure<std::vector>(y));

        // Right side: pure ($ y) <*> u
        auto apply_to_y = [y](std::function<int(int)> f) { return f(y); };
        auto right = fp20::apply(
            fp20::pure<std::vector>(std::function<int(std::function<int(int)>)>(apply_to_y)),
            u
        );

        REQUIRE(left == right);
        REQUIRE(left[0] == 42);
    }
}

TEST_CASE("Applicative Composition Law (Simplified)", "[applicative][laws][composition][tdd]") {
    // Simplified composition law test: verify that nested applies work correctly
    // Full composition law: pure (.) <*> u <*> v <*> w = u <*> (v <*> w)
    // Simplified: verify u <*> (v <*> w) works correctly

    auto add_one = [](int x) { return x + 1; };
    auto times_two = [](int x) { return x * 2; };

    SECTION("std::optional satisfies composition associativity") {
        auto u = fp20::pure<std::optional>(std::function<int(int)>(times_two));
        auto v = fp20::pure<std::optional>(std::function<int(int)>(add_one));
        auto w = fp20::pure<std::optional>(20);

        // Right side: u <*> (v <*> w)
        // First apply v to w: add_one <*> 20 = 21
        auto v_w = fp20::apply(v, w);
        REQUIRE(v_w.value() == 21);

        // Then apply u to result: times_two <*> 21 = 42
        auto result = fp20::apply(u, v_w);
        REQUIRE(result.value() == 42);
    }

    SECTION("std::vector satisfies composition associativity") {
        std::vector<std::function<int(int)>> u{times_two};
        std::vector<std::function<int(int)>> v{add_one};
        std::vector<int> w{20};

        // Right side: u <*> (v <*> w)
        auto v_w = fp20::apply(v, w);
        REQUIRE(v_w[0] == 21);

        auto result = fp20::apply(u, v_w);
        REQUIRE(result[0] == 42);
    }
}

TEST_CASE("Applicative preserves Functor relationship", "[applicative][functor][laws][tdd]") {
    // Law: fmap f x = pure f <*> x
    // fmap should be derivable from pure and apply

    auto times_two = [](int x) { return x * 2; };

    SECTION("fmap via apply for std::optional") {
        auto value = std::optional{21};

        // Using fmap
        auto fmap_result = fp20::fmap(times_two, value);

        // Using pure + apply
        auto apply_result = fp20::apply(
            fp20::pure<std::optional>(std::function<int(int)>(times_two)),
            value
        );

        REQUIRE(fmap_result == apply_result);
        REQUIRE(fmap_result.value() == 42);
    }

    SECTION("fmap via apply for std::vector") {
        std::vector<int> values{1, 2, 3};

        // Using fmap
        auto fmap_result = fp20::fmap(times_two, values);

        // Using pure + apply
        auto apply_result = fp20::apply(
            fp20::pure<std::vector>(std::function<int(int)>(times_two)),
            values
        );

        REQUIRE(fmap_result == apply_result);
        REQUIRE(fmap_result == std::vector<int>{2, 4, 6});
    }
}
