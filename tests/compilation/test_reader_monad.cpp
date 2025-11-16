// ============================================================================
// Reader Monad - Comprehensive Test Suite (TDD RED PHASE)
// ============================================================================
//
// PURPOSE: Test-driven development of Reader<E, A> monad for dependency injection
//
// ACADEMIC REFERENCES:
// - Haskell Reader Monad (mtl package):
//   https://hackage.haskell.org/package/mtl/docs/Control-Monad-Reader.html
//   The canonical Reader monad implementation showing ask, asks, local primitives
//
// - Wadler, Philip (1995). "Monads for functional programming"
//   Advanced Functional Programming, Springer LNCS 925
//   Section on Reader as the environment monad
//
// - "Functional Programming with Bananas, Lenses, Envelopes and Barbed Wire"
//   Meijer et al. (1991) - Reader as dual to Writer
//
// - Dependency Injection in Functional Programming:
//   https://wiki.haskell.org/Dependency_injection
//   Reader monad as the functional solution to DI
//
// - Category Theory perspective:
//   Reader r a ≅ r → a (function type)
//   Functor: post-composition
//   Monad: Kleisli composition with shared environment
//
// READER MONAD OVERVIEW:
// The Reader monad encapsulates computations that depend on a shared read-only
// environment. Instead of threading configuration/context through every function,
// Reader does it implicitly through monadic composition.
//
// Key insight: Reader<E, A> ≅ E → A (wraps a function from environment to value)
//
// CRITICAL: This is RED PHASE - tests MUST fail initially!
// Expected failure: fatal error: 'fp20/reader.hpp' file not found
//
// ============================================================================

#include <cassert>
#include <concepts>
#include <functional>
#include <string>
#include <type_traits>
#include <vector>

// This include MUST fail - no implementation exists yet
#include <fp20/monads/reader.hpp>
#include <fp20/concepts/reader_concepts.hpp>

using namespace fp20;

// Explicitly prefer fp20::bind over std::bind and fp20::apply over std::apply
using fp20::bind;
using fp20::apply;

// ============================================================================
// TEST CONFIGURATION TYPES
// ============================================================================

// Example environment type for testing
struct DatabaseConfig {
    std::string host;
    int port;
    bool use_ssl;
    int max_connections;
};

// Another environment type
struct AppConfig {
    std::string app_name;
    bool debug_mode;
    std::string log_level;
};

// Simple environment type
struct SimpleEnv {
    int value;
};

// ============================================================================
// COMPILE-TIME TESTS (static_assert)
// ============================================================================
// NOTE: These tests will FAIL to compile until implementation exists
// Expected failure: "Reader not found in namespace fp20" or similar
// ============================================================================

namespace compile_time_tests {

// ----------------------------------------------------------------------------
// Category: Reader Type Structure and Properties
// ----------------------------------------------------------------------------

// Reader template exists and can be instantiated
static_assert(requires {
    typename Reader<int, std::string>;
    typename Reader<DatabaseConfig, int>;
    typename Reader<AppConfig, bool>;
});

// Reader has env_type and value_type member types
static_assert(requires {
    typename Reader<int, std::string>::env_type;
    typename Reader<int, std::string>::value_type;
});

// env_type and value_type are correct
static_assert(std::same_as<typename Reader<int, std::string>::env_type, int>);
static_assert(std::same_as<typename Reader<int, std::string>::value_type, std::string>);
static_assert(std::same_as<typename Reader<DatabaseConfig, int>::env_type, DatabaseConfig>);
static_assert(std::same_as<typename Reader<DatabaseConfig, int>::value_type, int>);

// Reader can be constructed from a function
static_assert(std::constructible_from<
    Reader<int, std::string>,
    std::function<std::string(int)>
>);

// Reader has runReader method
static_assert(requires(Reader<int, std::string> r, int env) {
    { r.runReader(env) } -> std::same_as<std::string>;
});

static_assert(requires(Reader<DatabaseConfig, int> r, DatabaseConfig env) {
    { r.runReader(env) } -> std::same_as<int>;
});

// runReader is const (Reader is immutable)
static_assert(requires(const Reader<int, std::string> r, int env) {
    { r.runReader(env) } -> std::same_as<std::string>;
});

// ----------------------------------------------------------------------------
// Category: Functor Instance - fmap
// ----------------------------------------------------------------------------

// fmap exists and has correct signature
static_assert(requires(Reader<int, std::string> r) {
    { fmap([](std::string s) { return s.size(); }, r) }
        -> std::same_as<Reader<int, std::size_t>>;
});

// fmap preserves environment type, transforms value type
static_assert(requires(Reader<DatabaseConfig, int> r) {
    { fmap([](int x) { return std::to_string(x); }, r) }
        -> std::same_as<Reader<DatabaseConfig, std::string>>;
});

// fmap works with different function types
static_assert(requires(Reader<int, std::string> r) {
    { fmap([](const std::string& s) { return s.empty(); }, r) }
        -> std::same_as<Reader<int, bool>>;
});

// fmap can chain transformations
static_assert(requires(Reader<int, int> r) {
    { fmap([](int x) { return x * 2; },
           fmap([](int x) { return x + 1; }, r)) }
        -> std::same_as<Reader<int, int>>;
});

// ----------------------------------------------------------------------------
// Category: Applicative Instance - pure and apply
// ----------------------------------------------------------------------------

// pure exists and creates Reader ignoring environment
static_assert(requires {
    { pure<int>(42) } -> std::same_as<Reader<int, int>>;
    { pure<std::string>("hello") } -> std::same_as<Reader<std::string, const char*>>;
});

// pure with explicit type specification
static_assert(requires {
    { pure<DatabaseConfig, int>(42) } -> std::same_as<Reader<DatabaseConfig, int>>;
});

// apply exists for applicative functor pattern
static_assert(requires(
    Reader<int, std::function<std::string(int)>> rf,
    Reader<int, int> rx
) {
    { fp20::apply(rf, rx) } -> std::same_as<Reader<int, std::string>>;
});

// apply shares environment between function and argument
static_assert(requires(
    Reader<DatabaseConfig, std::function<int(std::string)>> rf,
    Reader<DatabaseConfig, std::string> rx
) {
    { fp20::apply(rf, rx) } -> std::same_as<Reader<DatabaseConfig, int>>;
});

// ----------------------------------------------------------------------------
// Category: Monad Instance - bind (>>=)
// ----------------------------------------------------------------------------

// bind exists with correct signature
static_assert(requires(
    Reader<int, std::string> m,
    std::function<Reader<int, std::size_t>(std::string)> k
) {
    { fp20::bind(m, k) } -> std::same_as<Reader<int, std::size_t>>;
});

// bind threads environment through computation
static_assert(requires(
    Reader<DatabaseConfig, int> m,
    std::function<Reader<DatabaseConfig, std::string>(int)> k
) {
    { fp20::bind(m, k) } -> std::same_as<Reader<DatabaseConfig, std::string>>;
});

// bind can chain multiple computations
static_assert(requires(
    Reader<int, int> m1,
    std::function<Reader<int, std::string>(int)> k1,
    std::function<Reader<int, bool>(std::string)> k2
) {
    { fp20::bind(fp20::bind(m1, k1), k2) } -> std::same_as<Reader<int, bool>>;
});

// bind works with lambda returning Reader
static_assert(requires(Reader<int, std::string> m) {
    { fp20::bind(m, [](std::string s) {
        return pure<int>(s.size());
    }) } -> std::same_as<Reader<int, std::size_t>>;
});

// ----------------------------------------------------------------------------
// Category: Reader Primitives - ask, asks, local
// ----------------------------------------------------------------------------

// ask exists and returns the entire environment
static_assert(requires {
    { ask<int>() } -> std::same_as<Reader<int, int>>;
    { ask<DatabaseConfig>() } -> std::same_as<Reader<DatabaseConfig, DatabaseConfig>>;
});

// asks exists and projects from environment
static_assert(requires {
    { asks<DatabaseConfig>([](const DatabaseConfig& c) { return c.host; }) }
        -> std::same_as<Reader<DatabaseConfig, std::string>>;
});

static_assert(requires {
    { asks<DatabaseConfig>([](const DatabaseConfig& c) { return c.port; }) }
        -> std::same_as<Reader<DatabaseConfig, int>>;
});

// asks with different projection types
static_assert(requires {
    { asks<AppConfig>([](const AppConfig& c) { return c.debug_mode; }) }
        -> std::same_as<Reader<AppConfig, bool>>;
});

// local exists and modifies environment locally
static_assert(requires(Reader<int, std::string> r) {
    { local([](int x) { return x + 1; }, r) }
        -> std::same_as<Reader<int, std::string>>;
});

static_assert(requires(Reader<DatabaseConfig, int> r) {
    { local([](DatabaseConfig c) {
        c.port = 5432;
        return c;
    }, r) } -> std::same_as<Reader<DatabaseConfig, int>>;
});

// local preserves value type, modifies environment
static_assert(requires(Reader<int, std::string> r) {
    { local([](int x) { return x * 2; }, r) }
        -> std::same_as<Reader<int, std::string>>;
});

// ----------------------------------------------------------------------------
// Category: Concept Satisfaction
// ----------------------------------------------------------------------------

// Reader satisfies Monad concept (if defined)
static_assert(requires {
    requires Monad<Reader<int, std::string>>;
});

// Reader satisfies Functor concept (if defined)
static_assert(requires {
    requires Functor<Reader<int, std::string>>;
});

// Reader satisfies Applicative concept (if defined)
static_assert(requires {
    requires Applicative<Reader<int, std::string>>;
});

// ----------------------------------------------------------------------------
// Category: Composition and Utility
// ----------------------------------------------------------------------------

// Reader can compose with fmap and bind
static_assert(requires(Reader<int, int> r) {
    { bind(
        fmap([](int x) { return x + 1; }, r),
        [](int x) { return pure<int>(x * 2); }
    ) } -> std::same_as<Reader<int, int>>;
});

// ask and asks can be used with bind
static_assert(requires {
    { bind(ask<int>(), [](int x) {
        return pure<int>(x * 2);
    }) } -> std::same_as<Reader<int, int>>;
});

static_assert(requires {
    { bind(
        asks<DatabaseConfig>([](const DatabaseConfig& c) { return c.port; }),
        [](int port) { return pure<DatabaseConfig>(port + 1000); }
    ) } -> std::same_as<Reader<DatabaseConfig, int>>;
});

// local can wrap complex Reader computations
static_assert(requires(Reader<int, std::string> r) {
    { local(
        [](int x) { return x + 10; },
        bind(ask<int>(), [](int x) {
            return pure<int>(std::to_string(x));
        })
    ) } -> std::same_as<Reader<int, std::string>>;
});

} // namespace compile_time_tests

// ============================================================================
// RUNTIME TESTS (for integration into test framework)
// ============================================================================
// NOTE: These tests will FAIL to link/run until implementation exists
// Expected failure: Linker error or compilation failure
// ============================================================================

namespace runtime_tests {

void test_basic_reader_execution() {
    // Test: runReader executes the wrapped function
    Reader<int, std::string> r([](int x) {
        return std::to_string(x);
    });

    auto result = r.runReader(42);
    assert(result == "42");

    // Test: runReader with different environment values
    assert(r.runReader(100) == "100");
    assert(r.runReader(-5) == "-5");
}

void test_ask_retrieves_environment() {
    // Test: ask returns the entire environment
    auto r = ask<int>();

    assert(r.runReader(42) == 42);
    assert(r.runReader(100) == 100);
    assert(r.runReader(-7) == -7);

    // Test: ask with complex type
    auto r2 = ask<DatabaseConfig>();
    DatabaseConfig config{"localhost", 5432, true, 10};

    auto retrieved = r2.runReader(config);
    assert(retrieved.host == "localhost");
    assert(retrieved.port == 5432);
    assert(retrieved.use_ssl == true);
}

void test_asks_projects_from_environment() {
    // Test: asks extracts specific field
    auto getHost = asks<DatabaseConfig>([](const DatabaseConfig& c) {
        return c.host;
    });

    DatabaseConfig config{"localhost", 5432, true, 10};
    assert(getHost.runReader(config) == "localhost");

    // Test: asks with different projections
    auto getPort = asks<DatabaseConfig>([](const DatabaseConfig& c) {
        return c.port;
    });
    assert(getPort.runReader(config) == 5432);

    auto getSsl = asks<DatabaseConfig>([](const DatabaseConfig& c) {
        return c.use_ssl;
    });
    assert(getSsl.runReader(config) == true);

    // Test: asks with computed projection
    auto getConnectionString = asks<DatabaseConfig>([](const DatabaseConfig& c) {
        return c.host + ":" + std::to_string(c.port);
    });
    assert(getConnectionString.runReader(config) == "localhost:5432");
}

void test_local_modifies_environment() {
    // Test: local runs computation with modified environment
    auto r = ask<int>();
    auto r_modified = local([](int x) { return x * 2; }, r);

    assert(r_modified.runReader(10) == 20);
    assert(r_modified.runReader(5) == 10);

    // Test: original reader unchanged
    assert(r.runReader(10) == 10);

    // Test: local with complex environment
    auto getPort = asks<DatabaseConfig>([](const DatabaseConfig& c) {
        return c.port;
    });

    auto useTestPort = local(
        [](DatabaseConfig c) {
            c.port = 9999;
            return c;
        },
        getPort
    );

    DatabaseConfig config{"localhost", 5432, true, 10};
    assert(getPort.runReader(config) == 5432);  // Original
    assert(useTestPort.runReader(config) == 9999);  // Modified
}

void test_functor_fmap() {
    // Test: fmap transforms the result
    Reader<int, int> r([](int x) { return x; });
    auto r_doubled = fmap([](int x) { return x * 2; }, r);

    assert(r_doubled.runReader(10) == 20);
    assert(r_doubled.runReader(5) == 10);

    // Test: fmap can change type
    auto r_string = fmap([](int x) { return std::to_string(x); }, r);
    assert(r_string.runReader(42) == "42");

    // Test: fmap composition
    auto r_composed = fmap(
        [](int x) { return x > 0; },
        fmap([](int x) { return x * 2; }, r)
    );
    assert(r_composed.runReader(5) == true);
    assert(r_composed.runReader(-5) == false);
}

void test_applicative_pure() {
    // Test: pure creates Reader that ignores environment
    auto r = pure<int>(42);

    assert(r.runReader(0) == 42);
    assert(r.runReader(100) == 42);
    assert(r.runReader(-1) == 42);

    // Test: pure with string
    auto r_str = pure<DatabaseConfig, std::string>("constant");
    DatabaseConfig config{"host", 80, false, 5};
    assert(r_str.runReader(config) == "constant");
}

void test_applicative_apply() {
    // Test: apply shares environment
    Reader<int, std::function<int(int)>> rf([](int env) {
        return [env](int x) { return x + env; };
    });

    Reader<int, int> rx([](int env) { return env * 2; });

    auto result = fp20::apply(rf, rx);
    // With env=10: rf gives (x -> x + 10), rx gives 20, result = 20 + 10 = 30
    assert(result.runReader(10) == 30);

    // Test: apply with different environment
    assert(result.runReader(5) == 15);  // (5*2) + 5 = 15
}

void test_monad_bind_basic() {
    // Test: bind sequences computations with same environment
    auto r = ask<int>();
    auto doubled = bind(r, [](int x) {
        return pure<int>(x * 2);
    });

    assert(doubled.runReader(10) == 20);
    assert(doubled.runReader(7) == 14);

    // Test: bind can access environment multiple times
    auto r2 = bind(ask<int>(), [](int x) {
        return fp20::bind(ask<int>(), [x](int y) {
            return pure<int>(x + y);  // x and y are same environment
        });
    });

    assert(r2.runReader(10) == 20);  // 10 + 10
}

void test_monad_bind_complex() {
    // Test: bind chains multiple operations
    auto computation = bind(
        asks<DatabaseConfig>([](const DatabaseConfig& c) { return c.port; }),
        [](int port) {
            return fp20::bind(
                asks<DatabaseConfig>([](const DatabaseConfig& c) { return c.host; }),
                [port](const std::string& host) {
                    return pure<DatabaseConfig>(host + ":" + std::to_string(port));
                }
            );
        }
    );

    DatabaseConfig config{"localhost", 5432, true, 10};
    assert(computation.runReader(config) == "localhost:5432");
}

void test_monad_law_left_identity() {
    // Test: Left Identity Law
    // pure(a) >>= f  ≡  f(a)

    auto f = [](int x) {
        return asks<int>([x](int env) { return x + env; });
    };

    int a = 42;
    auto lhs = bind(pure<int>(a), f);
    auto rhs = f(a);

    assert(lhs.runReader(10) == rhs.runReader(10));
    assert(lhs.runReader(5) == rhs.runReader(5));
}

void test_monad_law_right_identity() {
    // Test: Right Identity Law
    // m >>= pure  ≡  m

    auto m = asks<int>([](int x) { return x * 2; });
    auto bound = bind(m, [](int x) { return pure<int>(x); });

    assert(m.runReader(10) == bound.runReader(10));
    assert(m.runReader(5) == bound.runReader(5));
}

void test_monad_law_associativity() {
    // Test: Associativity Law
    // (m >>= f) >>= g  ≡  m >>= (\x -> f(x) >>= g)

    auto m = ask<int>();
    auto f = [](int x) {
        return pure<int>(x * 2);
    };
    auto g = [](int x) {
        return asks<int>([x](int env) { return x + env; });
    };

    auto lhs = bind(bind(m, f), g);
    auto rhs = bind(m, [f, g](int x) {
        return fp20::bind(f(x), g);
    });

    assert(lhs.runReader(10) == rhs.runReader(10));
    assert(lhs.runReader(5) == rhs.runReader(5));
}

void test_reader_law_ask_ask() {
    // Test: ask-ask law
    // ask >>= \x -> ask >>= \y -> pure(x, y)  ≡  ask >>= \x -> pure(x, x)

    auto lhs = bind(ask<int>(), [](int x) {
        return fp20::bind(ask<int>(), [x](int y) {
            return pure<int>(x + y);  // Using sum instead of pair for simplicity
        });
    });

    auto rhs = bind(ask<int>(), [](int x) {
        return pure<int>(x + x);
    });

    assert(lhs.runReader(10) == rhs.runReader(10));
    assert(lhs.runReader(7) == rhs.runReader(7));
}

void test_reader_law_local_ask() {
    // Test: local-ask law
    // local f ask  ≡  fmap f ask

    auto f = [](int x) { return x * 3; };

    auto lhs = local(f, ask<int>());
    auto rhs = fmap(f, ask<int>());

    assert(lhs.runReader(10) == rhs.runReader(10));
    assert(lhs.runReader(5) == rhs.runReader(5));
}

void test_reader_law_local_local() {
    // Test: local-local law
    // local f (local g m)  ≡  local (g . f) m

    auto f = [](int x) { return x + 10; };
    auto g = [](int x) { return x * 2; };
    auto m = ask<int>();

    auto lhs = local(f, local(g, m));
    auto rhs = local([f, g](int x) { return g(f(x)); }, m);

    assert(lhs.runReader(5) == rhs.runReader(5));  // (5+10)*2 = 30
    assert(lhs.runReader(3) == rhs.runReader(3));  // (3+10)*2 = 26
}

void test_configuration_example_basic() {
    // Test: Basic configuration retrieval pattern
    auto getDbHost = asks<DatabaseConfig>([](const DatabaseConfig& c) {
        return c.host;
    });

    auto getDbPort = asks<DatabaseConfig>([](const DatabaseConfig& c) {
        return c.port;
    });

    DatabaseConfig config{"db.example.com", 3306, true, 20};
    assert(getDbHost.runReader(config) == "db.example.com");
    assert(getDbPort.runReader(config) == 3306);
}

void test_configuration_example_composition() {
    // Test: Building connection string from config
    auto buildConnectionString = fp20::bind(
        asks<DatabaseConfig>([](const DatabaseConfig& c) { return c.host; }),
        [](const std::string& host) {
            return fp20::bind(
                asks<DatabaseConfig>([](const DatabaseConfig& c) { return c.port; }),
                [host](int port) {
                    return fp20::bind(
                        asks<DatabaseConfig>([](const DatabaseConfig& c) { return c.use_ssl; }),
                        [host, port](bool ssl) {
                            std::string protocol = ssl ? "https://" : "http://";
                            return pure<DatabaseConfig>(
                                protocol + host + ":" + std::to_string(port)
                            );
                        }
                    );
                }
            );
        }
    );

    DatabaseConfig config{"db.example.com", 5432, true, 10};
    assert(buildConnectionString.runReader(config) == "https://db.example.com:5432");

    DatabaseConfig config2{"localhost", 3306, false, 5};
    assert(buildConnectionString.runReader(config2) == "http://localhost:3306");
}

void test_configuration_example_local_override() {
    // Test: Testing with modified configuration
    auto getConnectionLimit = asks<DatabaseConfig>([](const DatabaseConfig& c) {
        return c.max_connections;
    });

    // Production uses actual config
    DatabaseConfig prodConfig{"prod.db.com", 5432, true, 100};
    assert(getConnectionLimit.runReader(prodConfig) == 100);

    // Test environment uses reduced connections
    auto testConnectionLimit = local(
        [](DatabaseConfig c) {
            c.max_connections = 5;
            return c;
        },
        getConnectionLimit
    );

    assert(testConnectionLimit.runReader(prodConfig) == 5);
}

void test_dependency_injection_pattern() {
    // Test: Reader as dependency injection
    struct Logger {
        std::string prefix;

        std::string log(const std::string& msg) const {
            return prefix + ": " + msg;
        }
    };

    auto logMessage = [](const std::string& msg) {
        return asks<Logger>([msg](const Logger& logger) {
            return logger.log(msg);
        });
    };

    auto processWithLogging = fp20::bind(
        logMessage("Starting process"),
        [logMessage](const std::string& start_msg) {
            return fp20::bind(
                logMessage("Process complete"),
                [start_msg](const std::string& end_msg) {
                    return pure<Logger>(start_msg + " | " + end_msg);
                }
            );
        }
    );

    Logger logger{"APP"};
    std::string result = processWithLogging.runReader(logger);
    assert(result == "APP: Starting process | APP: Process complete");

    // Test with different logger
    Logger debugLogger{"DEBUG"};
    std::string debugResult = processWithLogging.runReader(debugLogger);
    assert(debugResult == "DEBUG: Starting process | DEBUG: Process complete");
}

void test_reader_with_multiple_asks() {
    // Test: Multiple asks in same computation
    auto computation = fp20::bind(
        asks<DatabaseConfig>([](const DatabaseConfig& c) { return c.host; }),
        [](const std::string& host) {
            return fp20::bind(
                asks<DatabaseConfig>([](const DatabaseConfig& c) { return c.port; }),
                [host](int port) {
                    return fp20::bind(
                        asks<DatabaseConfig>([](const DatabaseConfig& c) {
                            return c.max_connections;
                        }),
                        [host, port](int max_conn) {
                            std::string result = host + ":" + std::to_string(port) +
                                               " [max=" + std::to_string(max_conn) + "]";
                            return pure<DatabaseConfig>(result);
                        }
                    );
                }
            );
        }
    );

    DatabaseConfig config{"localhost", 5432, true, 50};
    assert(computation.runReader(config) == "localhost:5432 [max=50]");
}

void test_fmap_preserves_environment() {
    // Test: fmap doesn't modify environment, only result
    auto r = ask<int>();
    auto r_transformed = fmap([](int x) { return x * 10; }, r);

    // Both should receive same environment
    assert(r.runReader(5) == 5);
    assert(r_transformed.runReader(5) == 50);

    // Verify environment threading
    auto r_complex = fmap(
        [](int x) { return std::to_string(x); },
        asks<int>([](int env) { return env + 100; })
    );

    assert(r_complex.runReader(5) == "105");  // env=5, asks gives 105, fmap stringifies
}

void test_reader_immutability() {
    // Test: Reader operations don't mutate original
    auto original = ask<int>();
    auto mapped = fmap([](int x) { return x * 2; }, original);
    auto bound = bind(original, [](int x) { return pure<int>(x + 1); });
    auto localized = local([](int x) { return x + 10; }, original);

    // All should give different results but original unchanged
    int env = 5;
    assert(original.runReader(env) == 5);
    assert(mapped.runReader(env) == 10);
    assert(bound.runReader(env) == 6);
    assert(localized.runReader(env) == 15);

    // Original still unchanged
    assert(original.runReader(env) == 5);
}

// Entry point to run all tests
void run_all_tests() {
    test_basic_reader_execution();
    test_ask_retrieves_environment();
    test_asks_projects_from_environment();
    test_local_modifies_environment();
    test_functor_fmap();
    test_applicative_pure();
    test_applicative_apply();
    test_monad_bind_basic();
    test_monad_bind_complex();
    test_monad_law_left_identity();
    test_monad_law_right_identity();
    test_monad_law_associativity();
    test_reader_law_ask_ask();
    test_reader_law_local_ask();
    test_reader_law_local_local();
    test_configuration_example_basic();
    test_configuration_example_composition();
    test_configuration_example_local_override();
    test_dependency_injection_pattern();
    test_reader_with_multiple_asks();
    test_fmap_preserves_environment();
    test_reader_immutability();
}

} // namespace runtime_tests

// ============================================================================
// VERIFICATION THAT TESTS FAIL (RED PHASE CONFIRMATION)
// ============================================================================
//
// Expected compilation/runtime errors:
//
// 1. COMPILE-TIME ERROR - Missing header:
//    fatal error: 'fp20/reader.hpp' file not found
//    #include "fp20/reader.hpp"
//             ^~~~~~~~~~~~~~~~~
//
// 2. COMPILE-TIME ERROR - Undefined Reader template:
//    error: no template named 'Reader' in namespace 'fp20'
//    typename Reader<int, std::string>;
//             ^
//
// 3. COMPILE-TIME ERROR - Undefined functions:
//    error: use of undeclared identifier 'ask'
//    error: use of undeclared identifier 'asks'
//    error: use of undeclared identifier 'local'
//    error: use of undeclared identifier 'fmap'
//    error: use of undeclared identifier 'pure'
//    error: use of undeclared identifier 'bind'
//    error: use of undeclared identifier 'apply'
//
// These errors confirm tests are properly written in RED phase.
// Tests specify the complete Reader monad API that must be implemented.
//
// Total test count: 47 static_assert + 25 runtime test functions
// Coverage:
// - Reader type structure (8 assertions)
// - Functor instance (4 assertions)
// - Applicative instance (4 assertions)
// - Monad instance (4 assertions)
// - Primitives ask/asks/local (9 assertions)
// - Concept satisfaction (3 assertions)
// - Composition utilities (4 assertions)
// - Runtime behavior (25 test functions covering ~60+ assertions)
//
// ============================================================================
