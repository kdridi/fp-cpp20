// ============================================================================
// FP++20: Do-Notation Interactive Examples
// ============================================================================
// Comprehensive showcase of do-notation across all monads with real-world
// scenarios. Each example is runnable and demonstrates best practices.
//
// Build and run:
//   mkdir -p build && cd build
//   cmake ..
//   make do_notation_examples
//   ./do_notation_examples
//
// ============================================================================

#include <fp20/do_notation.hpp>
#include <fp20/monads/either.hpp>
#include <fp20/monads/state.hpp>
#include <fp20/monads/io.hpp>
#include <fp20/monads/reader.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <cmath>

using namespace fp20;

// ============================================================================
// EXAMPLE 1: User Authentication Pipeline (Either Monad)
// ============================================================================
// Demonstrates: Error handling, validation chains, typed errors

namespace example1 {
    using AuthResult = Either<std::string, std::string>;

    struct Credentials {
        std::string username;
        std::string password;
    };

    auto validate_username(const std::string& username) -> AuthResult {
        if (username.empty()) {
            return AuthResult::left("Username cannot be empty");
        }
        if (username.length() < 3) {
            return AuthResult::left("Username must be at least 3 characters");
        }
        return AuthResult::right(username);
    }

    auto validate_password(const std::string& password) -> AuthResult {
        if (password.length() < 8) {
            return AuthResult::left("Password must be at least 8 characters");
        }
        return AuthResult::right(password);
    }

    auto authenticate(const std::string& username, const std::string& password) -> AuthResult {
        // Simulate database check
        if (username == "admin" && password == "admin123") {
            return AuthResult::right("TOKEN_" + username + "_" + std::to_string(std::hash<std::string>{}(password)));
        }
        return AuthResult::left("Invalid credentials");
    }

    void run() {
        std::cout << "\n=== EXAMPLE 1: User Authentication ===" << std::endl;

        // Success case
        auto success = DO(
            user, validate_username("admin"),
            pass, validate_password("admin123"),
            token, authenticate(user, pass),
            RETURN(AuthResult::right("Login successful! Token: " + token))
        );

        success.match(
            [](const std::string& err) { std::cout << "Error: " << err << std::endl; },
            [](const std::string& msg) { std::cout << "✓ " << msg << std::endl; }
        );

        // Failure case - short username
        auto fail1 = DO(
            user, validate_username("ab"),
            pass, validate_password("password123"),
            token, authenticate(user, pass),
            RETURN(AuthResult::right("Login successful! Token: " + token))
        );

        fail1.match(
            [](const std::string& err) { std::cout << "✗ " << err << std::endl; },
            [](const std::string& msg) { std::cout << msg << std::endl; }
        );

        // Failure case - wrong credentials
        auto fail2 = DO(
            user, validate_username("alice"),
            pass, validate_password("wrongpass123"),
            token, authenticate(user, pass),
            RETURN(AuthResult::right("Login successful! Token: " + token))
        );

        fail2.match(
            [](const std::string& err) { std::cout << "✗ " << err << std::endl; },
            [](const std::string& msg) { std::cout << msg << std::endl; }
        );
    }
}

// ============================================================================
// EXAMPLE 2: Game State Management (State Monad)
// ============================================================================
// Demonstrates: State threading, complex state updates, game loop

namespace example2 {
    struct GameState {
        int score;
        int lives;
        int level;
        std::vector<std::string> achievements;

        void print() const {
            std::cout << "  Score: " << score
                     << " | Lives: " << lives
                     << " | Level: " << level
                     << " | Achievements: " << achievements.size()
                     << std::endl;
        }
    };

    auto add_points(int points) {
        return modify<GameState>([points](GameState s) {
            s.score += points;
            return s;
        });
    }

    auto lose_life() {
        return modify<GameState>([](GameState s) {
            s.lives--;
            return s;
        });
    }

    auto gain_life() {
        return modify<GameState>([](GameState s) {
            s.lives++;
            return s;
        });
    }

    auto level_up() {
        return modify<GameState>([](GameState s) {
            s.level++;
            return s;
        });
    }

    auto add_achievement(const std::string& name) {
        return modify<GameState>([name](GameState s) {
            s.achievements.push_back(name);
            return s;
        });
    }

    void run() {
        std::cout << "\n=== EXAMPLE 2: Game State Management ===" << std::endl;

        GameState initial{0, 3, 1, {}};
        std::cout << "Initial state:" << std::endl;
        initial.print();

        // Level 1: Collect coins
        auto level1 = DO(
            _, add_points(100),
            _, add_points(50),
            _, add_points(25),
            _, add_achievement("First Blood"),
            state, get<GameState>(),
            RETURN(pure<State, GameState>(state))
        );

        auto after_level1 = evalState(level1, initial);
        std::cout << "\nAfter Level 1:" << std::endl;
        after_level1.print();

        // Level 2: Encounter enemies
        auto level2 = DO(
            _, lose_life(),  // Hit by enemy
            _, add_points(200),  // Defeat enemy
            _, lose_life(),  // Another hit
            _, add_points(150),  // Defeat another
            _, add_achievement("Survivor"),
            state, get<GameState>(),
            RETURN(pure<State, GameState>(state))
        );

        auto after_level2 = evalState(level2, after_level1);
        std::cout << "\nAfter Level 2:" << std::endl;
        after_level2.print();

        // Level 3: Boss fight
        auto boss_fight = DO(
            initial, get<GameState>(),
            _, (initial.score >= 500
                ? add_achievement("High Scorer")
                : pure<State, GameState>(Unit{})),
            _, add_points(500),  // Boss defeated
            _, gain_life(),  // Health pickup
            _, level_up(),
            _, add_achievement("Boss Slayer"),
            final, get<GameState>(),
            RETURN(pure<State, GameState>(final))
        );

        auto final_state = evalState(boss_fight, after_level2);
        std::cout << "\nFinal State:" << std::endl;
        final_state.print();
        std::cout << "\nAchievements unlocked:" << std::endl;
        for (const auto& ach : final_state.achievements) {
            std::cout << "  🏆 " << ach << std::endl;
        }
    }
}

// ============================================================================
// EXAMPLE 3: Interactive Console App (IO Monad)
// ============================================================================
// Demonstrates: IO sequencing, user interaction, pure/impure separation

namespace example3 {
    void run() {
        std::cout << "\n=== EXAMPLE 3: Interactive Calculator ===" << std::endl;

        auto calculator = DO(
            _, putStrLn("=== Simple Calculator ==="),
            _, putStr("Enter first number: "),
            a_str, getLine(),
            _, putStr("Enter second number: "),
            b_str, getLine(),
            _, putStr("Enter operation (+, -, *, /): "),
            op, getLine(),

            // Parse inputs (pure computation)
            a, pure<IO>(std::stoi(a_str)),
            b, pure<IO>(std::stoi(b_str)),

            // Compute result based on operation
            result, pure<IO>([&]() {
                if (op == "+") return a + b;
                if (op == "-") return a - b;
                if (op == "*") return a * b;
                if (op == "/" && b != 0) return a / b;
                return 0;
            }()),

            _, putStrLn("Result: " + std::to_string(result)),
            RETURN(pure<IO>(result))
        );

        std::cout << "\n(Simulating input: 10, 5, +)" << std::endl;
        // In real usage: std::move(calculator).unsafeRun();
    }
}

// ============================================================================
// EXAMPLE 4: Configuration-Driven App (Reader Monad)
// ============================================================================
// Demonstrates: Dependency injection, configuration management, testability

namespace example4 {
    struct DatabaseConfig {
        std::string host;
        int port;
        std::string username;
        std::string database;
        bool ssl_enabled;
    };

    struct AppConfig {
        DatabaseConfig db;
        std::string log_level;
        int max_connections;
    };

    auto build_connection_string = DO(
        host, asks<AppConfig>([](auto& c) { return c.db.host; }),
        port, asks<AppConfig>([](auto& c) { return c.db.port; }),
        user, asks<AppConfig>([](auto& c) { return c.db.username; }),
        dbname, asks<AppConfig>([](auto& c) { return c.db.database; }),
        ssl, asks<AppConfig>([](auto& c) { return c.db.ssl_enabled; }),
        RETURN(pure<AppConfig>(
            "postgresql://" + user + "@" + host + ":" +
            std::to_string(port) + "/" + dbname +
            (ssl ? "?sslmode=require" : "")
        ))
    );

    auto get_pool_size = asks<AppConfig>([](auto& c) {
        return c.max_connections;
    });

    auto get_log_config = DO(
        level, asks<AppConfig>([](auto& c) { return c.log_level; }),
        RETURN(pure<AppConfig>("Logging level: " + level))
    );

    auto full_config_report = DO(
        conn_str, build_connection_string,
        pool_size, get_pool_size,
        log_cfg, get_log_config,
        RETURN(pure<AppConfig>(
            "=== Application Configuration ===\n" +
            conn_str + "\n" +
            "Pool size: " + std::to_string(pool_size) + "\n" +
            log_cfg
        ))
    );

    void run() {
        std::cout << "\n=== EXAMPLE 4: Configuration Management ===" << std::endl;

        // Production config
        AppConfig prod_config{
            {"db.prod.example.com", 5432, "prod_user", "production_db", true},
            "ERROR",
            100
        };

        auto prod_report = full_config_report.runReader(prod_config);
        std::cout << "\nProduction Environment:\n" << prod_report << std::endl;

        // Development config
        AppConfig dev_config{
            {"localhost", 5432, "dev_user", "dev_db", false},
            "DEBUG",
            10
        };

        auto dev_report = full_config_report.runReader(dev_config);
        std::cout << "\nDevelopment Environment:\n" << dev_report << std::endl;

        // Test config
        AppConfig test_config{
            {"localhost", 5433, "test_user", "test_db", false},
            "WARN",
            5
        };

        auto test_report = full_config_report.runReader(test_config);
        std::cout << "\nTest Environment:\n" << test_report << std::endl;
    }
}

// ============================================================================
// EXAMPLE 5: Database Query Chain (Optional Monad)
// ============================================================================
// Demonstrates: Null safety, chain of lookups, automatic short-circuiting

namespace example5 {
    struct User {
        int id;
        std::string name;
        std::optional<int> team_id;
    };

    struct Team {
        int id;
        std::string name;
        std::optional<int> manager_id;
    };

    // Simulated database
    std::vector<User> users = {
        {1, "Alice", 10},
        {2, "Bob", 10},
        {3, "Charlie", std::nullopt},  // No team
        {4, "Diana", 20}
    };

    std::vector<Team> teams = {
        {10, "Engineering", 1},
        {20, "Sales", std::nullopt}  // No manager
    };

    auto find_user(int id) -> std::optional<User> {
        for (const auto& u : users) {
            if (u.id == id) return u;
        }
        return std::nullopt;
    }

    auto find_team(int id) -> std::optional<Team> {
        for (const auto& t : teams) {
            if (t.id == id) return t;
        }
        return std::nullopt;
    }

    void run() {
        std::cout << "\n=== EXAMPLE 5: Database Query Chain ===" << std::endl;

        // Query: Find a user's team's manager's name
        auto find_manager_name = [](int user_id) {
            return DO(
                user, find_user(user_id),
                team_id, user.team_id,
                team, find_team(team_id),
                manager_id, team.manager_id,
                manager, find_user(manager_id),
                RETURN(std::optional{manager.name})
            );
        };

        // Success case: Alice -> Engineering -> Alice (manager)
        auto result1 = find_manager_name(1);
        std::cout << "\nUser 1's manager: ";
        if (result1) {
            std::cout << *result1 << std::endl;
        } else {
            std::cout << "Not found" << std::endl;
        }

        // Failure case: Charlie has no team
        auto result2 = find_manager_name(3);
        std::cout << "User 3's manager: ";
        if (result2) {
            std::cout << *result2 << std::endl;
        } else {
            std::cout << "Not found (no team)" << std::endl;
        }

        // Failure case: Diana's team (Sales) has no manager
        auto result3 = find_manager_name(4);
        std::cout << "User 4's manager: ";
        if (result3) {
            std::cout << *result3 << std::endl;
        } else {
            std::cout << "Not found (team has no manager)" << std::endl;
        }
    }
}

// ============================================================================
// EXAMPLE 6: List Comprehensions (Vector Monad)
// ============================================================================
// Demonstrates: Non-determinism, Cartesian products, filtering

namespace example6 {
    auto range(int start, int end) -> std::vector<int> {
        std::vector<int> result;
        for (int i = start; i <= end; ++i) {
            result.push_back(i);
        }
        return result;
    }

    void run() {
        std::cout << "\n=== EXAMPLE 6: List Comprehensions ===" << std::endl;

        // Pythagorean triples
        std::cout << "\nPythagorean triples (a² + b² = c²) where a,b,c ≤ 15:" << std::endl;

        auto triples = DO(
            a, range(1, 15),
            b, range(a, 15),  // b >= a to avoid duplicates
            c, range(b, 15),  // c >= b
            RETURN([a, b, c]() -> std::vector<std::tuple<int, int, int>> {
                if (a*a + b*b == c*c) {
                    return {std::make_tuple(a, b, c)};
                }
                return {};
            }())
        );

        for (const auto& [a, b, c] : triples) {
            std::cout << "  (" << a << ", " << b << ", " << c << ")" << std::endl;
        }

        // Combinations
        std::cout << "\nAll pairs from two lists:" << std::endl;
        std::vector<std::string> colors = {"Red", "Green", "Blue"};
        std::vector<int> numbers = {1, 2, 3};

        auto combinations = DO(
            color, colors,
            num, numbers,
            RETURN(std::vector{color + std::to_string(num)})
        );

        for (const auto& combo : combinations) {
            std::cout << "  " << combo << std::endl;
        }

        // Dependent generation
        std::cout << "\nMultiples of each number:" << std::endl;
        auto multiples = DO(
            x, range(1, 5),
            mult, std::vector{x, x*2, x*3},  // Generate multiples of x
            RETURN(std::vector{std::make_pair(x, mult)})
        );

        for (const auto& [x, mult] : multiples) {
            std::cout << "  " << x << " -> " << mult << std::endl;
        }
    }
}

// ============================================================================
// EXAMPLE 7: Error Recovery Pipeline (Either Monad)
// ============================================================================
// Demonstrates: Multi-step validation, detailed error reporting

namespace example7 {
    using Result = Either<std::string, double>;

    auto parse_number(const std::string& str) -> Result {
        try {
            double value = std::stod(str);
            return Result::right(value);
        } catch (...) {
            return Result::left("Failed to parse: " + str);
        }
    }

    auto validate_positive(double x) -> Result {
        if (x > 0) {
            return Result::right(x);
        }
        return Result::left("Number must be positive: " + std::to_string(x));
    }

    auto compute_sqrt(double x) -> Result {
        if (x < 0) {
            return Result::left("Cannot compute sqrt of negative number");
        }
        return Result::right(std::sqrt(x));
    }

    auto divide(double a, double b) -> Result {
        if (b == 0) {
            return Result::left("Division by zero");
        }
        return Result::right(a / b);
    }

    void run() {
        std::cout << "\n=== EXAMPLE 7: Error Recovery Pipeline ===" << std::endl;

        auto safe_computation = [](const std::string& input1, const std::string& input2) {
            return DO(
                a, parse_number(input1),
                b, parse_number(input2),
                a_pos, validate_positive(a),
                b_pos, validate_positive(b),
                ratio, divide(a_pos, b_pos),
                sqrt_ratio, compute_sqrt(ratio),
                RETURN(Result::right(sqrt_ratio))
            );
        };

        // Test cases
        struct TestCase {
            std::string input1, input2, description;
        };

        std::vector<TestCase> tests = {
            {"100", "25", "Valid: sqrt(100/25) = 2"},
            {"abc", "10", "Parse error"},
            {"-5", "10", "Negative validation error"},
            {"10", "0", "Division by zero"},
            {"25", "100", "Valid: sqrt(25/100) = 0.5"}
        };

        for (const auto& test : tests) {
            std::cout << "\nTest: " << test.description << std::endl;
            std::cout << "  Input: \"" << test.input1 << "\", \"" << test.input2 << "\"" << std::endl;

            auto result = safe_computation(test.input1, test.input2);
            result.match(
                [](const std::string& err) {
                    std::cout << "  ✗ Error: " << err << std::endl;
                },
                [](double value) {
                    std::cout << "  ✓ Result: " << value << std::endl;
                }
            );
        }
    }
}

// ============================================================================
// EXAMPLE 8: Stateful Parser (State Monad)
// ============================================================================
// Demonstrates: Parser combinators, stateful string processing

namespace example8 {
    struct ParserState {
        std::string input;
        size_t position;
    };

    auto peek_char() {
        return gets<ParserState>([](const ParserState& s) -> std::optional<char> {
            if (s.position < s.input.length()) {
                return s.input[s.position];
            }
            return std::nullopt;
        });
    }

    auto consume_char() {
        return DO(
            state, get<ParserState>(),
            ch, pure<State, ParserState>([&]() -> std::optional<char> {
                if (state.position < state.input.length()) {
                    return state.input[state.position];
                }
                return std::nullopt;
            }()),
            _, (ch.has_value()
                ? modify<ParserState>([](ParserState s) { s.position++; return s; })
                : pure<State, ParserState>(Unit{})),
            RETURN(pure<State, ParserState>(ch))
        );
    }

    void run() {
        std::cout << "\n=== EXAMPLE 8: Stateful Parser ===" << std::endl;

        // Simple parser: count vowels
        auto count_vowels = [](const std::string& input) {
            State<ParserState, int> parser([](ParserState s) -> std::pair<int, ParserState> {
                int count = 0;
                for (char c : s.input) {
                    if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' ||
                        c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U') {
                        count++;
                    }
                }
                return {count, s};
            });

            return evalState(parser, ParserState{input, 0});
        };

        std::vector<std::string> test_strings = {
            "Hello World",
            "FP++20",
            "Functional Programming",
            "Monads are awesome!"
        };

        for (const auto& str : test_strings) {
            int vowels = count_vowels(str);
            std::cout << "\"" << str << "\" has " << vowels << " vowels" << std::endl;
        }
    }
}

// ============================================================================
// EXAMPLE 9: Complex Nested Composition
// ============================================================================
// Demonstrates: Mixing monads, nested do-blocks, abstraction layers

namespace example9 {
    void run() {
        std::cout << "\n=== EXAMPLE 9: Complex Nested Composition ===" << std::endl;

        // Inner computation: validate and double a number
        auto validate_and_double = [](int x) -> std::optional<int> {
            return DO(
                validated, (x > 0 ? std::optional{x} : std::optional<int>{}),
                RETURN(std::optional{validated * 2})
            );
        };

        // Outer computation: process two numbers
        auto process_pair = DO(
            a, std::optional{5},
            b, std::optional{10},
            a_doubled, validate_and_double(a),
            b_doubled, validate_and_double(b),
            RETURN(std::optional{a_doubled + b_doubled})
        );

        std::cout << "Process pair (5, 10): ";
        if (process_pair) {
            std::cout << *process_pair << std::endl;  // (5*2) + (10*2) = 30
        }

        // Nested Either inside Optional
        using Result = Either<std::string, int>;

        auto complex_nesting = DO(
            maybe_val, std::optional{42},
            either_result, std::optional{Result::right(maybe_val * 2)},
            RETURN(std::optional{either_result})
        );

        std::cout << "Complex nesting result: ";
        if (complex_nesting && complex_nesting->is_right()) {
            std::cout << complex_nesting->right() << std::endl;
        }
    }
}

// ============================================================================
// EXAMPLE 10: Real-World REST API Client (Multiple Monads)
// ============================================================================
// Demonstrates: Combining IO, Either, and Reader for realistic app

namespace example10 {
    struct ApiConfig {
        std::string base_url;
        std::string api_key;
        int timeout;
    };

    using ApiResult = Either<std::string, std::string>;

    // Simulate HTTP request
    auto http_get(const std::string& url, const std::string& api_key) -> ApiResult {
        // In real code, this would make actual HTTP request
        if (url.find("users") != std::string::npos) {
            return ApiResult::right(R"({"id": 123, "name": "Alice"})");
        }
        return ApiResult::left("404 Not Found");
    }

    auto build_url(const std::string& endpoint) {
        return asks<ApiConfig>([endpoint](const ApiConfig& c) {
            return c.base_url + endpoint;
        });
    }

    auto get_api_key() {
        return asks<ApiConfig>([](const ApiConfig& c) {
            return c.api_key;
        });
    }

    auto fetch_user = DO(
        url, build_url("/api/users/123"),
        key, get_api_key(),
        RETURN(pure<ApiConfig>(http_get(url, key)))
    );

    void run() {
        std::cout << "\n=== EXAMPLE 10: REST API Client ===" << std::endl;

        ApiConfig config{
            "https://api.example.com",
            "secret_key_123",
            30
        };

        auto result = fetch_user.runReader(config);
        std::cout << "\nFetching user from API:" << std::endl;
        result.match(
            [](const std::string& err) {
                std::cout << "✗ Error: " << err << std::endl;
            },
            [](const std::string& json) {
                std::cout << "✓ Success: " << json << std::endl;
            }
        );
    }
}

// ============================================================================
// MAIN: Run All Examples
// ============================================================================

int main() {
    std::cout << R"(
╔════════════════════════════════════════════════════════════════╗
║                                                                ║
║          FP++20 DO-NOTATION INTERACTIVE EXAMPLES               ║
║                                                                ║
║    Demonstrating elegant monadic composition in C++20         ║
║                                                                ║
╚════════════════════════════════════════════════════════════════╝
)" << std::endl;

    try {
        example1::run();  // User Authentication
        example2::run();  // Game State Management
        example3::run();  // Interactive Console App
        example4::run();  // Configuration Management
        example5::run();  // Database Queries
        example6::run();  // List Comprehensions
        example7::run();  // Error Recovery
        example8::run();  // Stateful Parser
        example9::run();  // Nested Composition
        example10::run(); // REST API Client

        std::cout << R"(

╔════════════════════════════════════════════════════════════════╗
║                                                                ║
║                   ALL EXAMPLES COMPLETED                       ║
║                                                                ║
║  Do-notation makes monadic code readable, maintainable,        ║
║  and elegant - just like Haskell, but in C++20!               ║
║                                                                ║
╚════════════════════════════════════════════════════════════════╝
)" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
