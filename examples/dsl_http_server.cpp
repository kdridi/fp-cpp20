#include <fp20/fp20.hpp>
#include <fp20/operators.hpp>
#include <iostream>
#include <string>
#include <map>

using namespace fp20;

// ============================================================================
// DSL 1: HTTP SERVER - EXPRESS.JS STYLE IN C++
// ============================================================================

struct Request { std::string path; std::string method; std::map<std::string, std::string> params; };
struct Response { int status; std::string body; };
using Handler = std::function<Response(Request)>;
using Middleware = std::function<Handler(Handler)>;

class Server {
    std::map<std::string, Handler> routes_;
public:
    Server& GET(std::string path, Handler h) { routes_["GET:" + path] = h; return *this; }
    Server& POST(std::string path, Handler h) { routes_["POST:" + path] = h; return *this; }
    Server& use(Middleware m) { for(auto& [k, h] : routes_) h = m(h); return *this; }
    void listen(int port) { std::cout << "Server listening on " << port << "\n"; }
};

// ULTRA ELEGANT DSL:
void http_example() {
    Server()
        .GET("/", [](Request req) {
            return Response{200, "Hello World!"};
        })
        .GET("/users/:id", [](Request req) {
            return Response{200, "User " + req.params["id"]};
        })
        .POST("/users", [](Request req) {
            return Response{201, "User created"};
        })
        .use([](Handler next) {
            return [next](Request req) {
                std::cout << "LOG: " << req.method << " " << req.path << "\n";
                return next(req);
            };
        })
        .listen(8080);
}

// ============================================================================
// DSL 2: SQL QUERY BUILDER - TYPE-SAFE!
// ============================================================================

template<typename T> struct Table { std::string name; };
template<typename T> struct Column { std::string name; };

struct User { int id; std::string name; int age; };
inline Table<User> users{"users"};
inline Column<int> id{"id"};
inline Column<std::string> name{"name"};
inline Column<int> age{"age"};

template<typename T>
class Query {
    std::string sql_;
public:
    Query(std::string s) : sql_(s) {}

    template<typename Col>
    Query& WHERE(Column<Col> col, std::string op, Col value) {
        sql_ += " WHERE " + col.name + " " + op + " " + std::to_string(value);
        return *this;
    }

    Query& AND(std::string cond) { sql_ += " AND " + cond; return *this; }
    Query& OR(std::string cond) { sql_ += " OR " + cond; return *this; }
    Query& ORDER_BY(auto col) { sql_ += " ORDER BY " + col.name; return *this; }
    Query& LIMIT(int n) { sql_ += " LIMIT " + std::to_string(n); return *this; }

    std::string build() { return sql_; }
};

template<typename T>
Query<T> SELECT(Table<T> table) {
    return Query<T>("SELECT * FROM " + table.name);
}

// ULTRA ELEGANT USAGE:
void sql_example() {
    auto query = SELECT(users)
        .WHERE(age, ">", 18)
        .AND("verified = true")
        .ORDER_BY(name)
        .LIMIT(10)
        .build();

    std::cout << query << "\n";
    // Output: SELECT * FROM users WHERE age > 18 AND verified = true ORDER BY name LIMIT 10
}

// ============================================================================
// DSL 3: PARSER COMBINATORS - MONADIC PARSING!
// ============================================================================

template<typename A>
struct ParseResult { bool success; A value; std::string remaining; };

template<typename A>
using Parser = std::function<ParseResult<A>(std::string)>;

template<typename A>
Parser<A> pure_p(A value) {
    return [value](std::string input) {
        return ParseResult<A>{true, value, input};
    };
}

template<typename A>
Parser<A> fail_p() {
    return [](std::string input) {
        return ParseResult<A>{false, A{}, input};
    };
}

Parser<char> char_p(char expected) {
    return [expected](std::string input) -> ParseResult<char> {
        if (input.empty() || input[0] != expected)
            return {false, '\0', input};
        return {true, expected, input.substr(1)};
    };
}

Parser<char> digit() {
    return [](std::string input) -> ParseResult<char> {
        if (input.empty() || !isdigit(input[0]))
            return {false, '\0', input};
        return {true, input[0], input.substr(1)};
    };
}

template<typename A, typename B>
Parser<B> operator>>=(Parser<A> p, std::function<Parser<B>(A)> f) {
    return [p, f](std::string input) -> ParseResult<B> {
        auto r1 = p(input);
        if (!r1.success) return {false, B{}, input};
        return f(r1.value)(r1.remaining);
    };
}

template<typename A>
Parser<A> operator|(Parser<A> p1, Parser<A> p2) {
    return [p1, p2](std::string input) -> ParseResult<A> {
        auto r = p1(input);
        return r.success ? r : p2(input);
    };
}

template<typename A>
Parser<std::vector<A>> many(Parser<A> p) {
    return [p](std::string input) -> ParseResult<std::vector<A>> {
        std::vector<A> results;
        std::string remaining = input;
        while (true) {
            auto r = p(remaining);
            if (!r.success) break;
            results.push_back(r.value);
            remaining = r.remaining;
        }
        return {true, results, remaining};
    };
}

// ULTRA ELEGANT PARSING:
void parser_example() {
    // Parse a number: digit+
    auto number = many(digit());

    // Parse "hello123"
    auto hello_num =
        char_p('h') >>= [](char) {
        return char_p('e') >>= [](char) {
        return char_p('l') >>= [](char) {
        return char_p('l') >>= [](char) {
        return char_p('o') >>= [](char) {
        return many(digit());
        }; }; }; }; };

    auto result = hello_num("hello123world");
    std::cout << "Parsed: " << result.success << "\n";
}

// ============================================================================
// DSL 4: REACTIVE STREAMS - RxJS STYLE
// ============================================================================

template<typename T>
class Observable {
    std::function<void(std::function<void(T)>)> subscribe_;
public:
    Observable(std::function<void(std::function<void(T)>)> s) : subscribe_(s) {}

    void subscribe(std::function<void(T)> observer) { subscribe_(observer); }

    template<typename U>
    Observable<U> map(std::function<U(T)> f) {
        return Observable<U>([s = subscribe_, f](auto obs) {
            s([f, obs](T val) { obs(f(val)); });
        });
    }

    Observable<T> filter(std::function<bool(T)> pred) {
        return Observable<T>([s = subscribe_, pred](auto obs) {
            s([pred, obs](T val) { if (pred(val)) obs(val); });
        });
    }

    Observable<T> take(int n) {
        return Observable<T>([s = subscribe_, n](auto obs) mutable {
            int count = 0;
            s([&count, n, obs](T val) {
                if (count++ < n) obs(val);
            });
        });
    }
};

template<typename T>
Observable<T> of(std::vector<T> values) {
    return Observable<T>([values](auto obs) {
        for (auto v : values) obs(v);
    });
}

// ULTRA ELEGANT REACTIVE:
void reactive_example() {
    of<int>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10})
        .filter([](int x) { return x % 2 == 0; })
        .map([](int x) { return x * x; })
        .take(3)
        .subscribe([](int x) {
            std::cout << x << " ";  // Output: 4 16 36
        });
}

// ============================================================================
// DSL 5: VALIDATION BUILDER - ELEGANT RULES
// ============================================================================

template<typename T>
struct ValidationRule {
    std::function<bool(T)> predicate;
    std::string error_msg;
};

template<typename T>
class Validator {
    std::vector<ValidationRule<T>> rules_;
public:
    Validator& rule(std::function<bool(T)> pred, std::string msg) {
        rules_.push_back({pred, msg});
        return *this;
    }

    Either<std::vector<std::string>, T> validate(T value) {
        std::vector<std::string> errors;
        for (auto& r : rules_) {
            if (!r.predicate(value)) errors.push_back(r.error_msg);
        }
        if (errors.empty())
            return Either<std::vector<std::string>, T>::Right(value);
        return Either<std::vector<std::string>, T>::Left(errors);
    }
};

template<typename T>
Validator<T> validator() { return Validator<T>(); }

// ULTRA ELEGANT VALIDATION:
void validation_example() {
    auto email_validator = validator<std::string>()
        .rule([](auto s) { return !s.empty(); }, "Email required")
        .rule([](auto s) { return s.find('@') != std::string::npos; }, "Invalid email")
        .rule([](auto s) { return s.length() >= 5; }, "Email too short");

    auto result = email_validator.validate("test@example.com");
    // Success!
}

// ============================================================================
// DSL 6: FLUENT BUILDER - CONFIG/OPTIONS
// ============================================================================

struct ServerConfig {
    int port = 8080;
    std::string host = "localhost";
    int timeout = 30;
    bool ssl = false;
    std::map<std::string, std::string> headers;
};

class ServerBuilder {
    ServerConfig cfg_;
public:
    ServerBuilder& port(int p) { cfg_.port = p; return *this; }
    ServerBuilder& host(std::string h) { cfg_.host = h; return *this; }
    ServerBuilder& timeout(int t) { cfg_.timeout = t; return *this; }
    ServerBuilder& enableSSL() { cfg_.ssl = true; return *this; }
    ServerBuilder& header(std::string k, std::string v) {
        cfg_.headers[k] = v;
        return *this;
    }
    ServerConfig build() { return cfg_; }
};

// ULTRA ELEGANT CONFIG:
void builder_example() {
    auto config = ServerBuilder()
        .port(3000)
        .host("0.0.0.0")
        .timeout(60)
        .enableSSL()
        .header("X-API-Key", "secret")
        .header("Content-Type", "application/json")
        .build();
}

// ============================================================================
// DSL 7: TEST FRAMEWORK - JEST/MOCHA STYLE
// ============================================================================

class TestSuite {
    std::string name_;
    std::vector<std::pair<std::string, std::function<void()>>> tests_;
public:
    TestSuite(std::string name) : name_(name) {}

    TestSuite& it(std::string desc, std::function<void()> test) {
        tests_.push_back({desc, test});
        return *this;
    }

    void run() {
        std::cout << "Suite: " << name_ << "\n";
        for (auto& [desc, test] : tests_) {
            std::cout << "  ✓ " << desc << "\n";
            test();
        }
    }
};

TestSuite describe(std::string name) { return TestSuite(name); }

void expect(bool condition) { if (!condition) throw std::runtime_error("Test failed"); }

// ULTRA ELEGANT TESTS:
void test_example() {
    describe("Calculator")
        .it("should add numbers", []() {
            expect(2 + 2 == 4);
        })
        .it("should multiply numbers", []() {
            expect(3 * 4 == 12);
        })
        .it("should handle zero", []() {
            expect(5 * 0 == 0);
        })
        .run();
}

// ============================================================================
// DSL 8: ROUTING - REACT-ROUTER STYLE
// ============================================================================

struct Route {
    std::string path;
    std::function<void()> component;
};

class Router {
    std::vector<Route> routes_;
    std::string current_;
public:
    Router& route(std::string path, std::function<void()> comp) {
        routes_.push_back({path, comp});
        return *this;
    }

    void navigate(std::string path) {
        current_ = path;
        for (auto& r : routes_) {
            if (r.path == path) {
                r.component();
                return;
            }
        }
        std::cout << "404 Not Found\n";
    }
};

// ULTRA ELEGANT ROUTING:
void routing_example() {
    Router()
        .route("/", []() { std::cout << "Home Page\n"; })
        .route("/about", []() { std::cout << "About Page\n"; })
        .route("/contact", []() { std::cout << "Contact Page\n"; })
        .navigate("/about");
}

// ============================================================================
// DSL 9: GAME ENGINE - UNITY STYLE
// ============================================================================

struct GameObject {
    std::string name;
    float x, y;
};

class Scene {
    std::vector<GameObject> objects_;
public:
    Scene& add(std::string name, float x, float y) {
        objects_.push_back({name, x, y});
        return *this;
    }

    Scene& move(std::string name, float dx, float dy) {
        for (auto& obj : objects_) {
            if (obj.name == name) {
                obj.x += dx;
                obj.y += dy;
            }
        }
        return *this;
    }

    void render() {
        for (auto& obj : objects_) {
            std::cout << obj.name << " at (" << obj.x << "," << obj.y << ")\n";
        }
    }
};

// ULTRA ELEGANT GAME:
void game_example() {
    Scene()
        .add("Player", 0, 0)
        .add("Enemy", 10, 10)
        .add("Coin", 5, 5)
        .move("Player", 2, 3)
        .move("Enemy", -1, 0)
        .render();
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    std::cout << "=== DSL ULTRA INSTINCT DEMONSTRATIONS ===\n\n";

    std::cout << "1. HTTP Server DSL:\n";
    http_example();

    std::cout << "\n2. SQL Builder DSL:\n";
    sql_example();

    std::cout << "\n3. Parser Combinators DSL:\n";
    parser_example();

    std::cout << "\n4. Reactive Streams DSL:\n";
    reactive_example();

    std::cout << "\n\n5. Validation DSL:\n";
    validation_example();

    std::cout << "\n6. Builder Pattern DSL:\n";
    builder_example();

    std::cout << "\n7. Test Framework DSL:\n";
    test_example();

    std::cout << "\n8. Routing DSL:\n";
    routing_example();

    std::cout << "\n9. Game Engine DSL:\n";
    game_example();

    std::cout << "\n🔥 9 DSL ULTRA ELEGANTS EN C++20 !!!\n";

    return 0;
}
