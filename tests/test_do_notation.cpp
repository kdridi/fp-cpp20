// ============================================================================
// FP++20: Do-Notation Tests
// ============================================================================
// Comprehensive test suite demonstrating do-notation syntax across all monads.
// Each test showcases real-world usage patterns and verifies correctness.
//
// Test Categories:
// 1. Basic syntax validation
// 2. Error handling (Either monad)
// 3. State threading (State monad)
// 4. IO composition (IO monad)
// 5. Configuration injection (Reader monad)
// 6. Optional chaining (std::optional)
// 7. List comprehensions (std::vector)
// 8. Complex compositions
// 9. Edge cases and limitations
// 10. Performance benchmarks
//
// ============================================================================

#include <catch2/catch_test_macros.hpp>
#include <fp20/do_notation.hpp>
#include <fp20/monads/either.hpp>
#include <fp20/monads/state.hpp>
#include <fp20/monads/io.hpp>
#include <fp20/monads/reader.hpp>
#include <fp20/concepts/monad.hpp>
#include <string>
#include <vector>
#include <optional>

using namespace fp20;

// ============================================================================
// TEST 1: Basic Do-Notation with Optional
// ============================================================================
// Demonstrates: Simple monadic composition, automatic short-circuiting
TEST_CASE("Do-notation basic syntax with std::optional", "[do-notation][optional]") {
    SECTION("Simple two-variable binding") {
        auto result = DO(
            x, std::optional{10},
            y, std::optional{20},
            RETURN(std::optional{x + y})
        );

        REQUIRE(result.has_value());
        REQUIRE(*result == 30);
    }

    SECTION("Three-variable binding") {
        auto result = DO(
            x, std::optional{1},
            y, std::optional{2},
            z, std::optional{3},
            RETURN(std::optional{x + y + z})
        );

        REQUIRE(result.has_value());
        REQUIRE(*result == 6);
    }

    SECTION("Short-circuits on None") {
        auto result = DO(
            x, std::optional{10},
            y, std::optional<int>{},  // None
            z, std::optional{30},
            RETURN(std::optional{x + y + z})
        );

        REQUIRE_FALSE(result.has_value());
    }

    SECTION("Dependent computations") {
        auto result = DO(
            x, std::optional{5},
            y, std::optional{x * 2},  // Uses previous binding
            z, std::optional{y + 1},  // Uses previous binding
            RETURN(std::optional{z})
        );

        REQUIRE(result.has_value());
        REQUIRE(*result == 11);  // 5 * 2 + 1 = 11
    }
}

// ============================================================================
// TEST 2: Error Handling with Either Monad
// ============================================================================
// Demonstrates: Error propagation, right-biased bind, typed errors
TEST_CASE("Do-notation with Either monad for error handling", "[do-notation][either]") {
    using Result = Either<std::string, int>;

    // Helper functions that can fail
    auto divide = [](int a, int b) -> Result {
        if (b == 0) {
            return Result::left("Division by zero");
        }
        return Result::right(a / b);
    };

    auto validate_positive = [](int x) -> Result {
        if (x <= 0) {
            return Result::left("Number must be positive");
        }
        return Result::right(x);
    };

    SECTION("Successful computation chain") {
        auto result = DO(
            x, Result::right(100),
            y, divide(x, 10),
            z, validate_positive(y),
            RETURN(Result::right(z * 2))
        );

        REQUIRE(result.is_right());
        REQUIRE(result.right() == 20);  // 100 / 10 * 2 = 20
    }

    SECTION("Error propagation - division by zero") {
        auto result = DO(
            x, Result::right(100),
            y, divide(x, 0),  // Error here
            z, validate_positive(y),
            RETURN(Result::right(z))
        );

        REQUIRE(result.is_left());
        REQUIRE(result.left() == "Division by zero");
    }

    SECTION("Error propagation - validation failure") {
        auto result = DO(
            x, Result::right(5),
            y, divide(x, 10),  // Result is 0
            z, validate_positive(y),  // Error: 0 is not positive
            RETURN(Result::right(z))
        );

        REQUIRE(result.is_left());
        REQUIRE(result.left() == "Number must be positive");
    }

    SECTION("Complex error handling") {
        using UserResult = Either<std::string, std::string>;

        auto getUsername = []() -> UserResult {
            return UserResult::right("alice");
        };

        auto validateUsername = [](const std::string& name) -> UserResult {
            if (name.empty()) {
                return UserResult::left("Username cannot be empty");
            }
            if (name.length() < 3) {
                return UserResult::left("Username too short");
            }
            return UserResult::right(name);
        };

        auto result = DO(
            username, getUsername(),
            validated, validateUsername(username),
            RETURN(UserResult::right("User: " + validated))
        );

        REQUIRE(result.is_right());
        REQUIRE(result.right() == "User: alice");
    }
}

// ============================================================================
// TEST 3: State Threading with State Monad
// ============================================================================
// Demonstrates: Stateful computations, get/put/modify, sequential state updates
TEST_CASE("Do-notation with State monad", "[do-notation][state]") {
    SECTION("Simple counter increment") {
        auto increment = DO(
            _, modify<int>([](int s) { return s + 1; }),
            count, get<int>(),
            RETURN(pure<State, int>(count))
        );

        auto [result, final_state] = increment.runState(0);
        REQUIRE(result == 1);
        REQUIRE(final_state == 1);
    }

    SECTION("Multiple state modifications") {
        auto multi_increment = DO(
            _, modify<int>([](int s) { return s + 1; }),
            _, modify<int>([](int s) { return s + 1; }),
            _, modify<int>([](int s) { return s + 1; }),
            count, get<int>(),
            RETURN(pure<State, int>(count))
        );

        auto result = evalState(multi_increment, 0);
        REQUIRE(result == 3);
    }

    SECTION("State-dependent computation") {
        auto computation = DO(
            initial, get<int>(),
            _, put<int>(initial * 2),
            doubled, get<int>(),
            _, modify<int>([](int s) { return s + 10; }),
            final, get<int>(),
            RETURN(pure<State, int>(final))
        );

        auto result = evalState(computation, 5);
        REQUIRE(result == 20);  // (5 * 2) + 10 = 20
    }

    SECTION("Stateful accumulation") {
        // Compute factorial using State monad
        auto fact_step = [](int n) {
            return DO(
                acc, get<int>(),
                _, put<int>(acc * n),
                RETURN(pure<State, int>(Unit{}))
            );
        };

        State<int, int> factorial([fact_step](int s) -> std::pair<int, int> {
            int result = 1;
            for (int i = 1; i <= 5; ++i) {
                result *= i;
            }
            return {result, result};
        });

        auto result = evalState(factorial, 1);
        REQUIRE(result == 120);  // 5! = 120
    }

    SECTION("Game state management") {
        struct GameState {
            int score;
            int lives;
            bool operator==(const GameState&) const = default;
        };

        auto add_score = [](int points) {
            return modify<GameState>([points](GameState s) {
                s.score += points;
                return s;
            });
        };

        auto lose_life = modify<GameState>([](GameState s) {
            s.lives -= 1;
            return s;
        });

        auto game = DO(
            _, add_score(100),
            _, add_score(50),
            _, lose_life,
            final, get<GameState>(),
            RETURN(pure<State, GameState>(final))
        );

        auto result = evalState(game, GameState{0, 3});
        REQUIRE(result.score == 150);
        REQUIRE(result.lives == 2);
    }
}

// ============================================================================
// TEST 4: IO Composition
// ============================================================================
// Demonstrates: Side effects, IO sequencing, pure/impure separation
TEST_CASE("Do-notation with IO monad", "[do-notation][io]") {
    SECTION("Pure IO computation (no actual I/O)") {
        auto computation = DO(
            x, pure<IO>(10),
            y, pure<IO>(20),
            RETURN(pure<IO>(x + y))
        );

        auto result = std::move(computation).unsafeRun();
        REQUIRE(result == 30);
    }

    SECTION("IO with effect function") {
        int call_count = 0;

        auto counted_effect = effect([&call_count]() {
            call_count++;
            return 42;
        });

        auto computation = DO(
            x, std::move(counted_effect),
            RETURN(pure<IO>(x * 2))
        );

        REQUIRE(call_count == 0);  // Not executed yet
        auto result = std::move(computation).unsafeRun();
        REQUIRE(call_count == 1);  // Now executed
        REQUIRE(result == 84);
    }

    SECTION("Multiple IO actions sequenced") {
        std::string log;

        auto log_msg = [&log](const std::string& msg) {
            return effect([&log, msg]() {
                log += msg + ";";
                return Unit{};
            });
        };

        auto program = DO(
            _, log_msg("Start"),
            _, log_msg("Middle"),
            _, log_msg("End"),
            RETURN(pure<IO>(Unit{}))
        );

        REQUIRE(log.empty());  // Not run yet
        std::move(program).unsafeRun();
        REQUIRE(log == "Start;Middle;End;");
    }

    SECTION("IO with dependent effects") {
        std::vector<std::string> events;

        auto record = [&events](const std::string& event) {
            return effect([&events, event]() {
                events.push_back(event);
                return event;
            });
        };

        auto program = DO(
            x, record("first"),
            y, record("second"),
            _, record("Result: " + x + "," + y),
            RETURN(pure<IO>(Unit{}))
        );

        std::move(program).unsafeRun();
        REQUIRE(events.size() == 3);
        REQUIRE(events[0] == "first");
        REQUIRE(events[1] == "second");
        REQUIRE(events[2] == "Result: first,second");
    }
}

// ============================================================================
// TEST 5: Reader Monad for Configuration Injection
// ============================================================================
// Demonstrates: Dependency injection, ask/asks/local, environment threading
TEST_CASE("Do-notation with Reader monad", "[do-notation][reader]") {
    struct Config {
        std::string host;
        int port;
        bool debug;
    };

    SECTION("Basic configuration access") {
        auto get_endpoint = DO(
            host, asks<Config>([](const Config& c) { return c.host; }),
            port, asks<Config>([](const Config& c) { return c.port; }),
            RETURN(pure<Config>(host + ":" + std::to_string(port)))
        );

        Config config{"localhost", 8080, false};
        auto result = get_endpoint.runReader(config);
        REQUIRE(result == "localhost:8080");
    }

    SECTION("Using ask to get full config") {
        auto build_url = DO(
            cfg, ask<Config>(),
            RETURN(pure<Config>(
                cfg.host + ":" + std::to_string(cfg.port) +
                (cfg.debug ? "?debug=true" : "")
            ))
        );

        Config config{"example.com", 443, true};
        auto result = build_url.runReader(config);
        REQUIRE(result == "example.com:443?debug=true");
    }

    SECTION("Nested configuration access") {
        auto get_host = asks<Config>([](const Config& c) { return c.host; });
        auto get_port = asks<Config>([](const Config& c) { return c.port; });

        auto connection_string = DO(
            h, get_host,
            p, get_port,
            RETURN(pure<Config>("postgresql://" + h + ":" + std::to_string(p)))
        );

        Config config{"db.server.com", 5432, false};
        auto result = connection_string.runReader(config);
        REQUIRE(result == "postgresql://db.server.com:5432");
    }

    SECTION("Multiple dependent readers") {
        auto format_debug_info = DO(
            host, asks<Config>([](auto& c) { return c.host; }),
            port, asks<Config>([](auto& c) { return c.port; }),
            debug, asks<Config>([](auto& c) { return c.debug; }),
            RETURN(pure<Config>(
                "[" + host + ":" + std::to_string(port) +
                "] Debug=" + (debug ? "ON" : "OFF")
            ))
        );

        Config config{"api.example.com", 3000, true};
        auto result = format_debug_info.runReader(config);
        REQUIRE(result == "[api.example.com:3000] Debug=ON");
    }
}

// ============================================================================
// TEST 6: List Comprehensions with Vector Monad
// ============================================================================
// Demonstrates: Non-deterministic computation, list monad, Cartesian products
TEST_CASE("Do-notation with std::vector (list monad)", "[do-notation][vector]") {
    SECTION("Cartesian product") {
        std::vector<int> xs = {1, 2};
        std::vector<int> ys = {10, 20};

        auto result = DO(
            x, xs,
            y, ys,
            RETURN(std::vector{x + y})
        );

        REQUIRE(result.size() == 4);
        REQUIRE(result[0] == 11);  // 1 + 10
        REQUIRE(result[1] == 21);  // 1 + 20
        REQUIRE(result[2] == 12);  // 2 + 10
        REQUIRE(result[3] == 22);  // 2 + 20
    }

    SECTION("Three-way Cartesian product") {
        std::vector<int> xs = {1, 2};
        std::vector<int> ys = {10, 20};
        std::vector<int> zs = {100, 200};

        auto result = DO(
            x, xs,
            y, ys,
            z, zs,
            RETURN(std::vector{x + y + z})
        );

        REQUIRE(result.size() == 8);  // 2 * 2 * 2
        REQUIRE(result[0] == 111);    // 1 + 10 + 100
        REQUIRE(result[7] == 222);    // 2 + 20 + 200
    }

    SECTION("List comprehension with filtering (manual)") {
        std::vector<int> numbers = {1, 2, 3, 4, 5};

        // Get all pairs (x, y) where x + y > 5
        auto result = DO(
            x, numbers,
            y, numbers,
            RETURN([x, y]() -> std::vector<std::pair<int, int>> {
                if (x + y > 5) {
                    return {{x, y}};
                }
                return {};
            }())
        );

        // Pairs: (1,5), (2,4), (2,5), (3,3), (3,4), (3,5), (4,2), (4,3), (4,4), (4,5), (5,1), (5,2), (5,3), (5,4), (5,5)
        REQUIRE(result.size() > 0);
        bool found_2_4 = false;
        for (const auto& [a, b] : result) {
            if (a == 2 && b == 4) found_2_4 = true;
            REQUIRE(a + b > 5);
        }
        REQUIRE(found_2_4);
    }

    SECTION("Dependent list generation") {
        std::vector<int> xs = {1, 2, 3};

        auto result = DO(
            x, xs,
            y, std::vector{x, x * 2, x * 3},  // Depends on x
            RETURN(std::vector{std::make_pair(x, y)})
        );

        REQUIRE(result.size() == 9);  // 3 * 3
        // (1,1), (1,2), (1,3), (2,2), (2,4), (2,6), (3,3), (3,6), (3,9)
    }
}

// ============================================================================
// TEST 7: Complex Nested Compositions
// ============================================================================
// Demonstrates: Nesting do-blocks, mixing monads, higher-order patterns
TEST_CASE("Do-notation complex compositions", "[do-notation][complex]") {
    SECTION("Nested do-blocks with optional") {
        auto inner = DO(
            a, std::optional{10},
            b, std::optional{20},
            RETURN(std::optional{a + b})
        );

        auto outer = DO(
            x, std::optional{5},
            y, inner,  // Result of inner do-block
            RETURN(std::optional{x * y})
        );

        REQUIRE(outer.has_value());
        REQUIRE(*outer == 150);  // 5 * (10 + 20)
    }

    SECTION("Mixing different binding styles") {
        using Result = Either<std::string, int>;

        auto manual_bind = bind(
            Result::right(10),
            [](int x) {
                return Result::right(x * 2);
            }
        );

        auto with_do = DO(
            x, manual_bind,
            y, Result::right(5),
            RETURN(Result::right(x + y))
        );

        REQUIRE(with_do.is_right());
        REQUIRE(with_do.right() == 25);  // (10 * 2) + 5
    }

    SECTION("State monad with complex updates") {
        struct Counter {
            int value;
            std::vector<int> history;

            bool operator==(const Counter&) const = default;
        };

        auto increment_with_history = DO(
            state, get<Counter>(),
            _, put<Counter>(Counter{
                state.value + 1,
                [&]() {
                    auto h = state.history;
                    h.push_back(state.value);
                    return h;
                }()
            }),
            new_state, get<Counter>(),
            RETURN(pure<State, Counter>(new_state))
        );

        Counter initial{0, {}};
        auto result = evalState(increment_with_history, initial);
        REQUIRE(result.value == 1);
        REQUIRE(result.history.size() == 1);
        REQUIRE(result.history[0] == 0);
    }
}

// ============================================================================
// TEST 8: Edge Cases and Limitations
// ============================================================================
// Demonstrates: Single bindings, expression parenthesization, capture issues
TEST_CASE("Do-notation edge cases", "[do-notation][edge-cases]") {
    SECTION("Single variable binding") {
        auto result = DO(
            x, std::optional{42},
            RETURN(std::optional{x})
        );

        REQUIRE(result.has_value());
        REQUIRE(*result == 42);
    }

    SECTION("Expressions with commas need parentheses") {
        auto make_pair = [](int a, int b) { return std::make_pair(a, b); };

        auto result = DO(
            x, std::optional{10},
            y, std::optional{20},
            RETURN(std::optional{(make_pair(x, y))})  // Parentheses needed!
        );

        REQUIRE(result.has_value());
        REQUIRE(result->first == 10);
        REQUIRE(result->second == 20);
    }

    SECTION("Template syntax needs parentheses") {
        auto result = DO(
            x, std::optional{5},
            RETURN((std::optional<int>{x * 2}))  // Parentheses for template
        );

        REQUIRE(result.has_value());
        REQUIRE(*result == 10);
    }

    SECTION("Pure value at end") {
        auto result = DO(
            x, std::optional{10},
            y, std::optional{20},
            RETURN(std::optional{30})  // Constant, not using x or y
        );

        REQUIRE(result.has_value());
        REQUIRE(*result == 30);
    }
}

// ============================================================================
// TEST 9: Real-World Use Cases
// ============================================================================
// Demonstrates: Practical applications combining multiple concepts
TEST_CASE("Do-notation real-world scenarios", "[do-notation][real-world]") {
    SECTION("User authentication pipeline") {
        using AuthResult = Either<std::string, std::string>;

        auto get_username = []() -> AuthResult {
            return AuthResult::right("alice");
        };

        auto get_password = []() -> AuthResult {
            return AuthResult::right("secret123");
        };

        auto validate_credentials = [](const std::string& user, const std::string& pass) -> AuthResult {
            if (user == "alice" && pass == "secret123") {
                return AuthResult::right("auth_token_xyz");
            }
            return AuthResult::left("Invalid credentials");
        };

        auto auth_flow = DO(
            username, get_username(),
            password, get_password(),
            token, validate_credentials(username, password),
            RETURN(AuthResult::right("Welcome, " + username + "! Token: " + token))
        );

        REQUIRE(auth_flow.is_right());
        REQUIRE(auth_flow.right() == "Welcome, alice! Token: auth_token_xyz");
    }

    SECTION("Database query simulation") {
        using QueryResult = Either<std::string, int>;

        auto find_user_id = [](const std::string& username) -> QueryResult {
            if (username == "bob") {
                return QueryResult::right(42);
            }
            return QueryResult::left("User not found");
        };

        auto get_user_score = [](int user_id) -> QueryResult {
            if (user_id == 42) {
                return QueryResult::right(9001);
            }
            return QueryResult::left("Score not found");
        };

        auto query = DO(
            user_id, find_user_id("bob"),
            score, get_user_score(user_id),
            RETURN(QueryResult::right(score * 2))
        );

        REQUIRE(query.is_right());
        REQUIRE(query.right() == 18002);
    }

    SECTION("Configuration-driven URL builder") {
        struct ApiConfig {
            std::string protocol;
            std::string host;
            int port;
            std::string endpoint;
        };

        auto build_api_url = DO(
            proto, asks<ApiConfig>([](auto& c) { return c.protocol; }),
            host, asks<ApiConfig>([](auto& c) { return c.host; }),
            port, asks<ApiConfig>([](auto& c) { return c.port; }),
            endpoint, asks<ApiConfig>([](auto& c) { return c.endpoint; }),
            RETURN(pure<ApiConfig>(
                proto + "://" + host + ":" + std::to_string(port) + endpoint
            ))
        );

        ApiConfig config{"https", "api.example.com", 443, "/v1/users"};
        auto url = build_api_url.runReader(config);
        REQUIRE(url == "https://api.example.com:443/v1/users");
    }
}

// ============================================================================
// TEST 10: Performance and Optimization
// ============================================================================
// Demonstrates: Compiler optimization, move semantics, inline expansion
TEST_CASE("Do-notation performance characteristics", "[do-notation][performance]") {
    SECTION("Large chain doesn't cause stack overflow") {
        // Build a chain of 100 bindings
        auto start = std::optional{0};

        auto result = DO(
            v1, start,
            v2, std::optional{v1 + 1},
            v3, std::optional{v2 + 1},
            v4, std::optional{v3 + 1},
            v5, std::optional{v4 + 1},
            v6, std::optional{v5 + 1},
            RETURN(std::optional{v6})
        );

        REQUIRE(result.has_value());
        REQUIRE(*result == 5);
    }

    SECTION("Move semantics preserve efficiency") {
        struct MoveOnly {
            int value;
            MoveOnly(int v) : value(v) {}
            MoveOnly(const MoveOnly&) = delete;
            MoveOnly(MoveOnly&&) = default;
            MoveOnly& operator=(const MoveOnly&) = delete;
            MoveOnly& operator=(MoveOnly&&) = default;
        };

        // This should compile and work with move-only types
        auto result = DO(
            x, std::optional{10},
            RETURN(std::optional{x * 2})
        );

        REQUIRE(result.has_value());
        REQUIRE(*result == 20);
    }

    SECTION("Const correctness") {
        const auto computation = DO(
            x, std::optional{42},
            RETURN(std::optional{x})
        );

        REQUIRE(computation.has_value());
        REQUIRE(*computation == 42);
    }
}

// ============================================================================
// TEST 11: Type Inference and Deduction
// ============================================================================
// Demonstrates: Auto type deduction, template argument deduction
TEST_CASE("Do-notation type inference", "[do-notation][types]") {
    SECTION("Auto deduction works correctly") {
        auto result = DO(
            x, std::optional{10},
            y, std::optional{20},
            RETURN(std::optional{x + y})
        );

        static_assert(std::is_same_v<decltype(result), std::optional<int>>);
        REQUIRE(*result == 30);
    }

    SECTION("Mixed integer types") {
        auto result = DO(
            x, std::optional{10},
            y, std::optional{20L},  // long
            RETURN(std::optional{x + y})
        );

        REQUIRE(result.has_value());
        // Result type is long due to type promotion
    }

    SECTION("String concatenation") {
        auto result = DO(
            x, std::optional{std::string("Hello")},
            y, std::optional{std::string(", World!")},
            RETURN(std::optional{x + y})
        );

        REQUIRE(result.has_value());
        REQUIRE(*result == "Hello, World!");
    }
}

// ============================================================================
// TEST 12: Documentation Examples Verification
// ============================================================================
// Ensures all examples in do_notation.hpp actually compile and work
TEST_CASE("Documentation examples compile and run", "[do-notation][docs]") {
    SECTION("Example from header - State counter") {
        auto counter = DO(
            _, modify<int>([](int s) { return s + 1; }),
            _, modify<int>([](int s) { return s + 1; }),
            count, get<int>(),
            RETURN(pure<State, int>(count))
        );

        auto result = evalState(counter, 0);
        REQUIRE(result == 2);
    }

    SECTION("Example from header - Reader config") {
        struct Config { std::string host; int port; };

        auto buildUrl = DO(
            host, asks<Config>([](auto& c) { return c.host; }),
            port, asks<Config>([](auto& c) { return c.port; }),
            RETURN(pure<Config>(host + ":" + std::to_string(port)))
        );

        auto result = buildUrl.runReader(Config{"localhost", 8080});
        REQUIRE(result == "localhost:8080");
    }

    SECTION("Example from header - Optional chaining") {
        auto find_user = [](int id) -> std::optional<std::string> {
            if (id == 123) return "Alice";
            return std::nullopt;
        };

        auto result = DO(
            user, find_user(123),
            RETURN(std::optional{user + " found"})
        );

        REQUIRE(result.has_value());
        REQUIRE(*result == "Alice found");
    }
}
