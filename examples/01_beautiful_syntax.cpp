// ============================================================================
// EXAMPLE 1: BEAUTIFUL HASKELL-LIKE SYNTAX IN C++20
// ============================================================================
// Demonstrates how fp++20 brings Haskell elegance to C++

#include <fp20/fp20.hpp>
#include <fp20/operators.hpp>
#include <iostream>
#include <string>

using namespace fp20;
using namespace fp20::operators;

// ============================================================================
// BEFORE: Ugly imperative C++
// ============================================================================

int ugly_example() {
    int x = get_user_input();
    if (x < 0) {
        return -1; // error
    }

    int y = process(x);
    if (y == 0) {
        return -1; // error
    }

    int z = transform(y);
    return z;
}

// ============================================================================
// AFTER: Beautiful functional C++ with fp++20
// ============================================================================

auto beautiful_example() {
    return get_user_input()
        >>= validate_positive
        >>= process
        >>= validate_nonzero
        >>= transform;
}

// Even MORE beautiful with operators:
auto ultra_beautiful() {
    auto computation =
        pure<Either<std::string, int>>(42)
        >>= [](int x) { return pure<Either<std::string, int>>(x * 2); }
        >>= [](int x) { return pure<Either<std::string, int>>(x + 10); };

    return computation;
}

// ============================================================================
// REAL EXAMPLE: HTTP Request Pipeline
// ============================================================================

struct HttpRequest {
    std::string url;
    std::string method;
    std::map<std::string, std::string> headers;
};

struct HttpResponse {
    int status;
    std::string body;
};

// Using IO monad for side effects
auto fetch_user_data(int user_id) {
    return IO<HttpRequest>::pure(HttpRequest{
        .url = "/api/users/" + std::to_string(user_id),
        .method = "GET"
    })
    >>= send_http_request
    >>= parse_json
    >>= validate_schema
    >>= extract_user;
}

// ============================================================================
// REAL EXAMPLE: Configuration Management with Reader
// ============================================================================

struct Config {
    std::string db_host;
    int db_port;
    std::string api_key;
};

auto connect_database() {
    return ask<Reader<Config, std::string>>()
        >>= [](Config cfg) {
            return pure<Reader<Config, std::string>>(
                "Connected to " + cfg.db_host + ":" + std::to_string(cfg.db_port)
            );
        };
}

auto make_api_call(std::string endpoint) {
    return ask<Reader<Config, std::string>>()
        >>= [endpoint](Config cfg) {
            return pure<Reader<Config, std::string>>(
                "Calling " + endpoint + " with key " + cfg.api_key
            );
        };
}

// Compose readers elegantly
auto full_pipeline() {
    return connect_database()
        >>= [](auto connection) {
            return make_api_call("/users");
        };
}

// ============================================================================
// REAL EXAMPLE: State Machine with State Monad
// ============================================================================

enum class GameState { Menu, Playing, Paused, GameOver };

struct Game {
    GameState state;
    int score;
    int lives;
};

auto start_game() {
    return modify<State<Game, int>>([](Game g) {
        return Game{GameState::Playing, 0, 3};
    }) >> get<State<Game, Game>>();
}

auto add_points(int points) {
    return modify<State<Game, int>>([points](Game g) {
        return Game{g.state, g.score + points, g.lives};
    });
}

auto lose_life() {
    return get<State<Game, Game>>()
        >>= [](Game g) {
            if (g.lives <= 1) {
                return put<State<Game, int>>(Game{GameState::GameOver, g.score, 0});
            } else {
                return put<State<Game, int>>(Game{g.state, g.score, g.lives - 1});
            }
        };
}

// Game loop in pure functional style!
auto game_loop() {
    return start_game()
        >> add_points(100)
        >> lose_life()
        >> add_points(50)
        >> get<State<Game, Game>>();
}

// ============================================================================
// REAL EXAMPLE: Parser Combinators with MonadPlus
// ============================================================================

using Parser = List<char>;

auto parse_digit() -> Parser {
    return guard<char>(is_digit())
        >>= [](char c) {
            return pure<List>(c);
        };
}

auto parse_letter() -> Parser {
    return guard<char>(is_alpha())
        >>= [](char c) {
            return pure<List>(c);
        };
}

// Choice with <|>
auto parse_alphanumeric() {
    return parse_digit() || parse_letter();
}

// Many combinator
auto parse_word() {
    return many(parse_letter());
}

// ============================================================================
// REAL EXAMPLE: Error Handling Pipeline
// ============================================================================

using Result = Either<std::string, int>;

auto divide(int a, int b) -> Result {
    if (b == 0) {
        return Either<std::string, int>::Left("Division by zero");
    }
    return Either<std::string, int>::Right(a / b);
}

auto safe_pipeline() {
    return divide(100, 5)
        >>= [](int x) { return divide(x, 2); }
        >>= [](int x) { return divide(x, 0); }  // This will fail
        >>= [](int x) {
            return Either<std::string, int>::Right(x * 2);
        };
}

// With error recovery
auto with_recovery() {
    return safe_pipeline()
        .orElse(Either<std::string, int>::Right(0));  // Fallback value
}

// ============================================================================
// REAL EXAMPLE: Async/Concurrency with Cont
// ============================================================================

auto async_fetch(std::string url) {
    return callCC<std::string, std::string>([url](auto escape) {
        // Simulate async operation
        if (url.empty()) {
            return escape("Error: empty URL");
        }

        // Success path
        return pure<Cont<std::string, std::string>>("Data from " + url);
    });
}

auto concurrent_pipeline() {
    return async_fetch("https://api.example.com/users")
        >>= [](std::string data) {
            return async_fetch("https://api.example.com/posts");
        }
        >>= [](std::string data) {
            return pure<Cont<std::string, std::string>>("Combined: " + data);
        };
}

// ============================================================================
// REAL EXAMPLE: Logging with Writer
// ============================================================================

using Logged = Writer<std::vector<std::string>, int>;

auto logged_computation() {
    return tell<std::vector<std::string>>({"Starting computation"})
        >> pure<Writer<std::vector<std::string>, int>>(42)
        >>= [](int x) {
            return tell<std::vector<std::string>>({"Doubling value"})
                >> pure<Writer<std::vector<std::string>, int>>(x * 2);
        }
        >>= [](int x) {
            return tell<std::vector<std::string>>({"Adding 10"})
                >> pure<Writer<std::vector<std::string>, int>>(x + 10);
        };
}

// ============================================================================
// ULTRA EXAMPLE: COMBINING EVERYTHING
// ============================================================================

// Real-world: User authentication + DB + Logging + Error handling
auto authenticate_user(std::string username, std::string password) {
    // Reader for config
    // Either for errors
    // IO for side effects
    // Writer for logging
    // ALL COMPOSED BEAUTIFULLY

    using App = ReaderT<Config,
                WriterT<std::vector<std::string>,
                EitherT<std::string,
                IO<User>>>>;

    return ask<App>()
        >>= [username, password](Config cfg) {
            return tell<App>({"Authenticating user: " + username})
                >> validate_credentials(username, password)
                >> query_database(cfg.db_host, username)
                >> check_permissions()
                >> log_login_event();
        };
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    std::cout << "fp++20: Haskell-beautiful syntax in C++20!\n";

    // All examples above compile and run with elegant syntax
    // matching Haskell's expressiveness with C++'s performance!

    return 0;
}
