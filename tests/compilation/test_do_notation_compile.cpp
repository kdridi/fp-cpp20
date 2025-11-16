// ============================================================================
// FP++20: Do-Notation Compilation Test
// ============================================================================
// Verifies that do-notation syntax compiles correctly with all monads.
// This is a compilation-only test - if it compiles, it passes.
//
// ============================================================================

#include <fp20/do_notation.hpp>
#include <fp20/monads/either.hpp>
#include <fp20/monads/state.hpp>
#include <fp20/monads/io.hpp>
#include <fp20/monads/reader.hpp>
#include <optional>
#include <vector>
#include <string>

using namespace fp20;

// ============================================================================
// Test 1: DO with std::optional
// ============================================================================

void test_do_optional() {
    auto result = DO(
        x, std::optional{10},
        y, std::optional{20},
        RETURN(std::optional{x + y})
    );
    (void)result;
}

// ============================================================================
// Test 2: DO with Either monad
// ============================================================================

void test_do_either() {
    using Result = Either<std::string, int>;

    auto result = DO(
        x, Result::right(10),
        y, Result::right(20),
        RETURN(Result::right(x + y))
    );
    (void)result;
}

// ============================================================================
// Test 3: DO with State monad
// ============================================================================

void test_do_state() {
    auto result = DO(
        x, get<int>(),
        _, modify<int>([](int s) { return s + 1; }),
        y, get<int>(),
        RETURN(pure<State, int>(y))
    );
    (void)result;
}

// ============================================================================
// Test 4: DO with IO monad
// ============================================================================

void test_do_io() {
    auto result = DO(
        x, pure<IO>(10),
        y, pure<IO>(20),
        RETURN(pure<IO>(x + y))
    );
    (void)result;
}

// ============================================================================
// Test 5: DO with Reader monad
// ============================================================================

void test_do_reader() {
    struct Config { int value; };

    auto result = DO(
        x, asks<Config>([](const Config& c) { return c.value; }),
        RETURN(pure<Config>(x * 2))
    );
    (void)result;
}

// ============================================================================
// Test 6: DO with std::vector (list monad)
// ============================================================================

void test_do_vector() {
    auto result = DO(
        x, std::vector{1, 2, 3},
        y, std::vector{10, 20},
        RETURN(std::vector{x + y})
    );
    (void)result;
}

// ============================================================================
// Test 7: Nested DO blocks
// ============================================================================

void test_nested_do() {
    auto inner = DO(
        a, std::optional{10},
        RETURN(std::optional{a * 2})
    );

    auto outer = DO(
        x, std::optional{5},
        y, inner,
        RETURN(std::optional{x + y})
    );
    (void)outer;
}

// ============================================================================
// Test 8: Multiple bindings (6 variables)
// ============================================================================

void test_many_bindings() {
    auto result = DO(
        x1, std::optional{1},
        x2, std::optional{2},
        x3, std::optional{3},
        x4, std::optional{4},
        x5, std::optional{5},
        x6, std::optional{6},
        RETURN(std::optional{x1 + x2 + x3 + x4 + x5 + x6})
    );
    (void)result;
}

// ============================================================================
// Test 9: Dependent computations
// ============================================================================

void test_dependent() {
    auto result = DO(
        x, std::optional{5},
        y, std::optional{x * 2},    // Uses x
        z, std::optional{y + 1},    // Uses y
        RETURN(std::optional{z})
    );
    (void)result;
}

// ============================================================================
// Test 10: Complex types
// ============================================================================

void test_complex_types() {
    struct User {
        std::string name;
        int age;
    };

    auto result = DO(
        name, std::optional{std::string("Alice")},
        age, std::optional{30},
        RETURN(std::optional{User{name, age}})
    );
    (void)result;
}

// ============================================================================
// Main - All tests pass if this compiles
// ============================================================================

int main() {
    // If this file compiles, all do-notation syntax tests pass!
    return 0;
}
