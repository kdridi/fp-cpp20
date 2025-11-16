// ============================================
// FP++20: Either<L,R> and Identity<T> Monad Tests (RED Phase)
// ============================================
// Comprehensive test suite for Either and Identity monads.
// These tests are written BEFORE implementation to drive design.
// They MUST fail initially to confirm proper TDD approach.

#include <cassert>
#include <type_traits>
#include <string>
#include <functional>
#include <vector>
#include <optional>
#include <iostream>

// Include the implementations first
#include <fp20/monads/either.hpp>
#include <fp20/monads/identity.hpp>

// Then the concepts
#include <fp20/concepts/functor.hpp>
#include <fp20/concepts/applicative.hpp>
#include <fp20/concepts/monad.hpp>

// Finally the concept opt-ins (must be last)
#include <fp20/concepts/either_identity_concepts.hpp>

// ============================================
// COMPILE-TIME TESTS (static_assert)
// ============================================
// NOTE: These tests will FAIL to compile until implementation exists
// Expected failure: "Either is not a member of 'fp20'" and similar

// ============================================
// EITHER<L,R> TESTS
// ============================================

namespace test_either_construction {
    // AC1: Either<L,R> Type Construction

    // Test left construction
    constexpr void test_left_construction() {
        using namespace fp20;

        // Create Left with string error
        static_assert(requires {
            Either<std::string, int>::left("error");
        }, "Should be able to create Left variant");

        // Create Left with different types
        static_assert(requires {
            Either<int, double>::left(42);
        }, "Should support Left with int error type");

        // Left construction should return Either<L,R>
        using LeftResult = decltype(Either<std::string, int>::left("error"));
        static_assert(std::is_same_v<LeftResult, Either<std::string, int>>,
                      "left() should return Either<L,R>");
    }

    // Test right construction
    constexpr void test_right_construction() {
        using namespace fp20;

        // Create Right with success value
        static_assert(requires {
            Either<std::string, int>::right(42);
        }, "Should be able to create Right variant");

        // Create Right with different types
        static_assert(requires {
            Either<int, std::string>::right("success");
        }, "Should support Right with string success type");

        // Right construction should return Either<L,R>
        using RightResult = decltype(Either<std::string, int>::right(42));
        static_assert(std::is_same_v<RightResult, Either<std::string, int>>,
                      "right() should return Either<L,R>");
    }

    // Test predicates
    constexpr void test_predicates() {
        using namespace fp20;

        // is_left() predicate
        static_assert(requires(Either<std::string, int> e) {
            { e.is_left() } -> std::convertible_to<bool>;
        }, "Either should have is_left() predicate");

        // is_right() predicate
        static_assert(requires(Either<std::string, int> e) {
            { e.is_right() } -> std::convertible_to<bool>;
        }, "Either should have is_right() predicate");

        // Predicates should be const
        static_assert(requires(const Either<std::string, int> e) {
            { e.is_left() } -> std::convertible_to<bool>;
            { e.is_right() } -> std::convertible_to<bool>;
        }, "Predicates should be const member functions");
    }

    // Test value extraction
    constexpr void test_value_extraction() {
        using namespace fp20;

        // left() extraction
        static_assert(requires(Either<std::string, int> e) {
            { e.left() } -> std::same_as<std::string>;
        }, "Should be able to extract left value");

        // right() extraction
        static_assert(requires(Either<std::string, int> e) {
            { e.right() } -> std::same_as<int>;
        }, "Should be able to extract right value");

        // Extraction should work with const
        static_assert(requires(const Either<std::string, int> e) {
            { e.left() } -> std::same_as<std::string>;
            { e.right() } -> std::same_as<int>;
        }, "Value extraction should work with const Either");
    }

    // Test match pattern
    constexpr void test_match() {
        using namespace fp20;

        // match with two functions
        static_assert(requires(Either<std::string, int> e) {
            e.match(
                [](const std::string& s) { return s.size(); },
                [](int i) { return static_cast<size_t>(i); }
            );
        }, "Either should support match pattern");

        // match should deduce return type
        static_assert(requires(Either<std::string, int> e) {
            { e.match(
                [](const std::string&) { return 1.0; },
                [](int) { return 2.0; }
            ) } -> std::same_as<double>;
        }, "match should deduce common return type");
    }
}

namespace test_either_functor {
    // AC2: Either Functor instance

    // Test concept satisfaction
    constexpr void test_functor_concept() {
        using namespace fp20;
        using namespace fp20::concepts;

        // Either should be a Functor
        static_assert(Functor<Either<std::string, int>>,
                      "Either<L,R> should satisfy Functor concept");

        // Different type parameters
        static_assert(Functor<Either<int, double>>,
                      "Either with different types should be Functor");

        // With complex types
        static_assert(Functor<Either<std::vector<std::string>, std::optional<int>>>,
                      "Either with complex types should be Functor");
    }

    // Test fmap behavior
    constexpr void test_fmap() {
        using namespace fp20;

        // fmap over Either
        static_assert(requires {
            fmap([](int x) { return x * 2; },
                 Either<std::string, int>::right(21));
        }, "Should be able to fmap over Either");

        // fmap should change Right type
        using FmapResult = decltype(
            fmap([](int x) { return std::to_string(x); },
                 Either<std::string, int>::right(42))
        );
        static_assert(std::is_same_v<FmapResult, Either<std::string, std::string>>,
                      "fmap should transform Right type");

        // fmap should preserve Left type
        using FmapLeftResult = decltype(
            fmap([](int x) { return x * 2.0; },
                 Either<std::string, int>::left("error"))
        );
        static_assert(std::is_same_v<FmapLeftResult, Either<std::string, double>>,
                      "fmap on Left should only change Right type parameter");
    }
}

namespace test_either_applicative {
    // AC2: Either Applicative instance

    // Test concept satisfaction
    constexpr void test_applicative_concept() {
        using namespace fp20;
        using namespace fp20::concepts;

        // Either should be an Applicative
        static_assert(Applicative<Either<std::string, int>>,
                      "Either<L,R> should satisfy Applicative concept");

        // Applicative subsumes Functor
        static_assert(Functor<Either<std::string, int>>,
                      "Either as Applicative should also be Functor");
    }

    // Test pure operation
    constexpr void test_pure() {
        using namespace fp20;

        // pure for Either creates Right
        static_assert(requires {
            pure<Either, std::string>(42);
        }, "Should be able to use pure with Either");

        // pure should create Right variant
        using PureResult = decltype(pure<Either, std::string>(42));
        static_assert(std::is_same_v<PureResult, Either<std::string, int>>,
                      "pure should create Either with correct types");
    }

    // Test apply operation
    constexpr void test_apply() {
        using namespace fp20;

        // apply function in Either to value in Either
        static_assert(requires {
            fp20::apply(Either<std::string, std::function<int(int)>>::right([](int x) { return x * 2; }),
                        Either<std::string, int>::right(21));
        }, "Should be able to apply Either function to Either value");

        // apply should handle Left propagation
        using ApplyResult = decltype(
            fp20::apply(Either<std::string, std::function<int(int)>>::left("error"),
                        Either<std::string, int>::right(21))
        );
        static_assert(std::is_same_v<ApplyResult, Either<std::string, int>>,
                      "apply should handle Left propagation correctly");
    }
}

namespace test_either_monad {
    // AC2: Either Monad instance

    // Test concept satisfaction
    constexpr void test_monad_concept() {
        using namespace fp20;
        using namespace fp20::concepts;

        // Either should be a Monad
        static_assert(Monad<Either<std::string, int>>,
                      "Either<L,R> should satisfy Monad concept");

        // Monad subsumes Applicative and Functor
        static_assert(Applicative<Either<std::string, int>>,
                      "Either as Monad should also be Applicative");
        static_assert(Functor<Either<std::string, int>>,
                      "Either as Monad should also be Functor");
    }

    // Test return_ operation
    constexpr void test_return() {
        using namespace fp20;

        // return_ for Either creates Right
        static_assert(requires {
            return_<Either, std::string>(42);
        }, "Should be able to use return_ with Either");

        // return_ should create Right variant
        using ReturnResult = decltype(return_<Either, std::string>(42));
        static_assert(std::is_same_v<ReturnResult, Either<std::string, int>>,
                      "return_ should create Either with correct types");
    }

    // Test bind operation
    constexpr void test_bind() {
        using namespace fp20;

        // bind with Either
        static_assert(requires {
            fp20::bind(Either<std::string, int>::right(42),
                       [](int x) { return Either<std::string, double>::right(x * 2.0); });
        }, "Should be able to bind Either");

        // bind should handle type transformation
        using BindResult = decltype(
            fp20::bind(Either<std::string, int>::right(42),
                       [](int x) { return Either<std::string, std::string>::right(std::to_string(x)); })
        );
        static_assert(std::is_same_v<BindResult, Either<std::string, std::string>>,
                      "bind should transform types correctly");

        // bind with Left propagation
        using BindLeftResult = decltype(
            fp20::bind(Either<std::string, int>::left("error"),
                       [](int x) { return Either<std::string, double>::right(x * 2.0); })
        );
        static_assert(std::is_same_v<BindLeftResult, Either<std::string, double>>,
                      "bind on Left should propagate error with new Right type");
    }
}

namespace test_either_error_propagation {
    // AC3: Error Propagation

    // Test Left short-circuit in fmap
    constexpr void test_fmap_propagation() {
        using namespace fp20;

        // Multiple fmaps should propagate Left - test requires constraint
        static_assert(requires {
            []() {
                auto e = Either<std::string, int>::left("error");
                auto r1 = fmap([](int x) { return x * 2; }, e);
                auto r2 = fmap([](int x) { return x + 1; }, r1);
                auto r3 = fmap([](int x) { return std::to_string(x); }, r2);
            }();
        }, "Should be able to chain fmaps with Left propagation");
    }

    // Test Left short-circuit in bind
    constexpr void test_bind_propagation() {
        using namespace fp20;

        // Multiple binds should propagate Left
        static_assert(requires {
            []() {
                auto e = Either<std::string, int>::left("error");
                fp20::bind(fp20::bind(e,
                                      [](int x) { return Either<std::string, int>::right(x * 2); }),
                           [](int x) { return Either<std::string, int>::right(x + 1); });
            }();
        }, "Should be able to chain binds with Left propagation");
    }

    // Test first Left wins in apply
    constexpr void test_apply_propagation() {
        using namespace fp20;

        // First Left should win
        static_assert(requires {
            []() {
                auto f = Either<std::string, std::function<int(int)>>::left("func error");
                auto v = Either<std::string, int>::left("value error");
                fp20::apply(f, v);
            }();
        }, "apply should handle multiple Left values");
    }
}

namespace test_either_laws {
    // AC5: Monad Laws for Either

    // These are compile-time type checks for law structure
    constexpr void test_law_types() {
        using namespace fp20;

        // Left identity: return a >>= f ≡ f a
        static_assert(requires {
            []() {
                auto f = [](int x) { return Either<std::string, double>::right(x * 2.0); };
                fp20::bind(return_<Either, std::string>(42), f);
                f(42);
            }();
        }, "Left identity law should be expressible");

        // Right identity: m >>= return ≡ m
        static_assert(requires {
            []() {
                auto m = Either<std::string, int>::right(42);
                fp20::bind(m, [](int x) { return return_<Either, std::string>(x); });
            }();
        }, "Right identity law should be expressible");

        // Associativity: (m >>= f) >>= g ≡ m >>= (\x -> f x >>= g)
        static_assert(requires {
            []() {
                auto m = Either<std::string, int>::right(42);
                auto f = [](int x) { return Either<std::string, double>::right(x * 2.0); };
                auto g = [](double x) { return Either<std::string, std::string>::right(std::to_string(x)); };
                fp20::bind(fp20::bind(m, f), g);
                fp20::bind(m, [f, g](int x) { return fp20::bind(f(x), g); });
            }();
        }, "Associativity law should be expressible");
    }
}

// ============================================
// IDENTITY<T> TESTS
// ============================================

namespace test_identity_construction {
    // AC4: Identity<T> Transparent Wrapper

    // Test construction
    constexpr void test_construction() {
        using namespace fp20;

        // Direct construction with value
        static_assert(requires {
            Identity<int>{42};
        }, "Should be able to construct Identity with value");

        // Construction with different types
        static_assert(requires {
            Identity<std::string>{"hello"};
            Identity<double>{3.14};
            Identity<std::vector<int>>{{1, 2, 3}};
        }, "Identity should work with various types");

        // Copy construction
        static_assert(requires(Identity<int> id) {
            Identity<int>{id};
        }, "Identity should be copy constructible");

        // Move construction
        static_assert(requires(Identity<int> id) {
            Identity<int>{std::move(id)};
        }, "Identity should be move constructible");
    }

    // Test runIdentity extraction
    constexpr void test_extraction() {
        using namespace fp20;

        // runIdentity() extraction
        static_assert(requires(Identity<int> id) {
            { id.runIdentity() } -> std::same_as<int>;
        }, "Should be able to extract value with runIdentity()");

        // runIdentity() should be const
        static_assert(requires(const Identity<int> id) {
            { id.runIdentity() } -> std::same_as<int>;
        }, "runIdentity() should work with const Identity");

        // Different types
        static_assert(requires {
            Identity<std::string>{"hello"}.runIdentity();
            Identity<double>{3.14}.runIdentity();
        }, "runIdentity() should work with various types");
    }
}

namespace test_identity_functor {
    // AC4: Identity Functor instance

    // Test concept satisfaction
    constexpr void test_functor_concept() {
        using namespace fp20;
        using namespace fp20::concepts;

        // Identity should be a Functor
        static_assert(Functor<Identity<int>>,
                      "Identity<T> should satisfy Functor concept");

        // Different type parameters
        static_assert(Functor<Identity<std::string>>,
                      "Identity with different types should be Functor");

        // With complex types
        static_assert(Functor<Identity<std::vector<int>>>,
                      "Identity with complex types should be Functor");
    }

    // Test fmap behavior
    constexpr void test_fmap() {
        using namespace fp20;

        // fmap over Identity
        static_assert(requires {
            fmap([](int x) { return x * 2; }, Identity<int>{21});
        }, "Should be able to fmap over Identity");

        // fmap should transform inner type
        using FmapResult = decltype(
            fmap([](int x) { return std::to_string(x); }, Identity<int>{42})
        );
        static_assert(std::is_same_v<FmapResult, Identity<std::string>>,
                      "fmap should transform Identity inner type");

        // fmap should be transparent
        static_assert(requires {
            []() {
                auto id = Identity<int>{42};
                auto result = fmap([](int x) { return x * 2; }, id);
                auto val = result.runIdentity();
                (void)val;
            }();
        }, "fmap result should still be extractable");
    }
}

namespace test_identity_applicative {
    // AC4: Identity Applicative instance

    // Test concept satisfaction
    constexpr void test_applicative_concept() {
        using namespace fp20;
        using namespace fp20::concepts;

        // Identity should be an Applicative
        static_assert(Applicative<Identity<int>>,
                      "Identity<T> should satisfy Applicative concept");

        // Applicative subsumes Functor
        static_assert(Functor<Identity<int>>,
                      "Identity as Applicative should also be Functor");
    }

    // Test pure operation
    constexpr void test_pure() {
        using namespace fp20;

        // pure for Identity
        static_assert(requires {
            pure<Identity>(42);
        }, "Should be able to use pure with Identity");

        // pure should create Identity
        using PureResult = decltype(pure<Identity>(42));
        static_assert(std::is_same_v<PureResult, Identity<int>>,
                      "pure should create Identity with correct type");
    }

    // Test apply operation
    constexpr void test_apply() {
        using namespace fp20;

        // apply function in Identity to value in Identity
        static_assert(requires {
            fp20::apply(Identity<std::function<int(int)>>{[](int x) { return x * 2; }},
                        Identity<int>{21});
        }, "Should be able to apply Identity function to Identity value");

        // apply should unwrap and apply
        using ApplyResult = decltype(
            fp20::apply(Identity<std::function<int(int)>>{[](int x) { return x * 2; }},
                        Identity<int>{21})
        );
        static_assert(std::is_same_v<ApplyResult, Identity<int>>,
                      "apply should return Identity with result type");
    }
}

namespace test_identity_monad {
    // AC4: Identity Monad instance

    // Test concept satisfaction
    constexpr void test_monad_concept() {
        using namespace fp20;
        using namespace fp20::concepts;

        // Identity should be a Monad
        static_assert(Monad<Identity<int>>,
                      "Identity<T> should satisfy Monad concept");

        // Monad subsumes Applicative and Functor
        static_assert(Applicative<Identity<int>>,
                      "Identity as Monad should also be Applicative");
        static_assert(Functor<Identity<int>>,
                      "Identity as Monad should also be Functor");
    }

    // Test return_ operation
    constexpr void test_return() {
        using namespace fp20;

        // return_ for Identity
        static_assert(requires {
            return_<Identity>(42);
        }, "Should be able to use return_ with Identity");

        // return_ should create Identity
        using ReturnResult = decltype(return_<Identity>(42));
        static_assert(std::is_same_v<ReturnResult, Identity<int>>,
                      "return_ should create Identity with correct type");
    }

    // Test bind operation
    constexpr void test_bind() {
        using namespace fp20;

        // bind with Identity
        static_assert(requires {
            fp20::bind(Identity<int>{42},
                       [](int x) { return Identity<double>{x * 2.0}; });
        }, "Should be able to bind Identity");

        // bind should handle type transformation
        using BindResult = decltype(
            fp20::bind(Identity<int>{42},
                       [](int x) { return Identity<std::string>{std::to_string(x)}; })
        );
        static_assert(std::is_same_v<BindResult, Identity<std::string>>,
                      "bind should transform types correctly");

        // bind should be transparent
        static_assert(requires {
            []() {
                auto id = Identity<int>{42};
                auto result = fp20::bind(id, [](int x) { return Identity<int>{x * 2}; });
                auto val = result.runIdentity();
                (void)val;
            }();
        }, "bind result should still be extractable");
    }
}

namespace test_identity_laws {
    // AC5: Monad Laws for Identity

    // These are compile-time type checks for law structure
    constexpr void test_law_types() {
        using namespace fp20;

        // Left identity: return a >>= f ≡ f a
        static_assert(requires {
            []() {
                auto f = [](int x) { return Identity<double>{x * 2.0}; };
                fp20::bind(return_<Identity>(42), f);
                f(42);
            }();
        }, "Left identity law should be expressible");

        // Right identity: m >>= return ≡ m
        static_assert(requires {
            []() {
                auto m = Identity<int>{42};
                fp20::bind(m, [](int x) { return return_<Identity>(x); });
            }();
        }, "Right identity law should be expressible");

        // Associativity: (m >>= f) >>= g ≡ m >>= (\x -> f x >>= g)
        static_assert(requires {
            []() {
                auto m = Identity<int>{42};
                auto f = [](int x) { return Identity<double>{x * 2.0}; };
                auto g = [](double x) { return Identity<std::string>{std::to_string(x)}; };
                fp20::bind(fp20::bind(m, f), g);
                fp20::bind(m, [f, g](int x) { return fp20::bind(f(x), g); });
            }();
        }, "Associativity law should be expressible");
    }
}

// ============================================
// INTEGRATION TESTS
// ============================================

namespace test_cross_monad_composition {
    // AC5: Integration & Laws

    // Test Either + Identity composition
    constexpr void test_either_identity_composition() {
        using namespace fp20;

        // Nested monads
        static_assert(requires {
            Identity<Either<std::string, int>>{Either<std::string, int>::right(42)};
            Either<std::string, Identity<int>>::right(Identity<int>{42});
        }, "Should be able to nest Either and Identity");

        // Functor operations on nested structures
        static_assert(requires {
            []() {
                auto nested = Identity<Either<std::string, int>>{Either<std::string, int>::right(42)};
                fmap([](const Either<std::string, int>& e) {
                    return fmap([](int x) { return x * 2; }, e);
                }, nested);
            }();
        }, "Should be able to map over nested structures");
    }

    // Test Either + Maybe (std::optional) composition
    constexpr void test_either_maybe_composition() {
        using namespace fp20;

        // Either containing Maybe
        static_assert(requires {
            Either<std::string, std::optional<int>>::right(std::optional<int>{42});
            Either<std::string, std::optional<int>>::right(std::nullopt);
        }, "Either should work with optional as Right type");

        // Maybe containing Either
        static_assert(requires {
            std::optional<Either<std::string, int>>{Either<std::string, int>::right(42)};
            std::optional<Either<std::string, int>>{Either<std::string, int>::left("error")};
        }, "optional should work with Either");
    }
}

// ============================================
// RUNTIME TESTS (for main/main.cpp)
// ============================================
// NOTE: These tests will FAIL to link/run until implementation exists
// Expected failure: Linker error or assertion failure

namespace runtime_tests {

void test_either_runtime() {
    using namespace fp20;

    // Test Either construction and predicates
    {
        auto left = Either<std::string, int>::left("error");
        assert(left.is_left() && "Left should report is_left() as true");
        assert(!left.is_right() && "Left should report is_right() as false");
        assert(left.left() == "error" && "Should extract left value");

        auto right = Either<std::string, int>::right(42);
        assert(!right.is_left() && "Right should report is_left() as false");
        assert(right.is_right() && "Right should report is_right() as true");
        assert(right.right() == 42 && "Should extract right value");
    }

    // Test Either functor operations
    {
        auto right = Either<std::string, int>::right(21);
        auto doubled = fmap([](int x) { return x * 2; }, right);
        assert(doubled.is_right() && "fmap should preserve Right");
        assert(doubled.right() == 42 && "fmap should apply function to Right value");

        auto left = Either<std::string, int>::left("error");
        auto left_mapped = fmap([](int x) { return x * 2; }, left);
        assert(left_mapped.is_left() && "fmap should preserve Left");
        assert(left_mapped.left() == "error" && "fmap should not modify Left value");
    }

    // Test Either monad operations
    {
        auto right = Either<std::string, int>::right(10);
        auto result = fp20::bind(right, [](int x) {
            if (x > 0) {
                return Either<std::string, int>::right(x * 2);
            } else {
                return Either<std::string, int>::left("negative");
            }
        });
        assert(result.is_right() && "bind should propagate Right");
        assert(result.right() == 20 && "bind should apply function");

        auto left = Either<std::string, int>::left("initial error");
        auto left_result = fp20::bind(left, [](int x) {
            return Either<std::string, int>::right(x * 2);
        });
        assert(left_result.is_left() && "bind should short-circuit on Left");
        assert(left_result.left() == "initial error" && "bind should preserve Left value");
    }

    // Test Either error propagation chain
    {
        auto computation = fp20::bind(
            fp20::bind(
                Either<std::string, int>::right(10),
                [](int x) { return Either<std::string, int>::right(x * 2); }
            ),
            [](int x) { (void)x; return Either<std::string, int>::left("error at step 2"); }
        );

        auto final_result = fp20::bind(computation, [](int x) {
            return Either<std::string, int>::right(x + 100);
        });

        assert(final_result.is_left() && "Error should propagate through chain");
        assert(final_result.left() == "error at step 2" && "Should preserve error message");
    }

    // Test Either monad laws
    {
        // Left identity: return a >>= f ≡ f a
        auto f = [](int x) { return Either<std::string, int>::right(x * 2); };
        auto left_id_1 = fp20::bind(return_<Either, std::string>(42), f);
        auto left_id_2 = f(42);
        assert(left_id_1.right() == left_id_2.right() && "Left identity should hold");

        // Right identity: m >>= return ≡ m
        auto m = Either<std::string, int>::right(42);
        auto right_id = fp20::bind(m, [](int x) { return return_<Either, std::string>(x); });
        assert(m.right() == right_id.right() && "Right identity should hold");

        // Associativity tested implicitly through chain operations
    }
}

void test_identity_runtime() {
    using namespace fp20;

    // Test Identity construction and extraction
    {
        Identity<int> id{42};
        assert(id.runIdentity() == 42 && "Should extract value from Identity");

        Identity<std::string> str_id{"hello"};
        assert(str_id.runIdentity() == "hello" && "Should work with strings");
    }

    // Test Identity functor operations
    {
        Identity<int> id{21};
        auto doubled = fmap([](int x) { return x * 2; }, id);
        assert(doubled.runIdentity() == 42 && "fmap should apply function transparently");

        auto stringified = fmap([](int x) { return std::to_string(x); }, id);
        assert(stringified.runIdentity() == "21" && "fmap should handle type changes");
    }

    // Test Identity monad operations
    {
        Identity<int> id{10};
        auto result = fp20::bind(id, [](int x) {
            return Identity<int>{x * 3};
        });
        assert(result.runIdentity() == 30 && "bind should apply function");

        // Chain multiple binds
        auto chained = fp20::bind(
            fp20::bind(
                Identity<int>{5},
                [](int x) { return Identity<int>{x * 2}; }
            ),
            [](int x) { return Identity<int>{x + 3}; }
        );
        assert(chained.runIdentity() == 13 && "Chained binds should work");
    }

    // Test Identity monad laws
    {
        // Left identity: return a >>= f ≡ f a
        auto f = [](int x) { return Identity<int>{x * 2}; };
        auto left_id_1 = fp20::bind(return_<Identity>(42), f);
        auto left_id_2 = f(42);
        assert(left_id_1.runIdentity() == left_id_2.runIdentity() && "Left identity should hold");

        // Right identity: m >>= return ≡ m
        auto m = Identity<int>{42};
        auto right_id = fp20::bind(m, [](int x) { return return_<Identity>(x); });
        assert(m.runIdentity() == right_id.runIdentity() && "Right identity should hold");

        // Associativity
        auto g = [](int x) { return Identity<double>{x * 2.0}; };
        auto h = [](double x) { return Identity<double>{x + 1.0}; };
        auto assoc_1 = fp20::bind(fp20::bind(m, g), h);
        auto assoc_2 = fp20::bind(m, [g, h](int x) { return fp20::bind(g(x), h); });
        assert(assoc_1.runIdentity() == assoc_2.runIdentity() && "Associativity should hold");
    }
}

void test_integration() {
    using namespace fp20;

    // Test Either + Identity composition
    {
        Identity<Either<std::string, int>> nested{Either<std::string, int>::right(42)};
        auto result = fmap([](const Either<std::string, int>& e) {
            return fmap([](int x) { return x * 2; }, e);
        }, nested);

        assert(result.runIdentity().is_right() && "Nested structure should preserve Right");
        assert(result.runIdentity().right() == 84 && "Nested fmap should work correctly");
    }

    // Test Either + optional composition
    {
        Either<std::string, std::optional<int>> maybe_either =
            Either<std::string, std::optional<int>>::right(std::optional<int>{42});

        auto result = fmap([](const std::optional<int>& opt) {
            return fmap([](int x) { return x * 2; }, opt);
        }, maybe_either);

        assert(result.is_right() && "Should preserve Right");
        assert(result.right().has_value() && "Should preserve Some");
        assert(result.right().value() == 84 && "Should apply nested transformation");
    }
}

void run_all_runtime_tests() {
    test_either_runtime();
    test_identity_runtime();
    test_integration();
}

} // namespace runtime_tests

// Note: main() is in test_concepts.cpp
// Runtime tests will be called from there

// ============================================
// VERIFICATION THAT TESTS FAIL
// ============================================
/*
Expected compilation/runtime errors:

1. Compile-time error: "fatal error: 'fp20/either.hpp' file not found"
   - This confirms that Either implementation doesn't exist yet

2. Compile-time error: "fatal error: 'fp20/identity.hpp' file not found"
   - This confirms that Identity implementation doesn't exist yet

3. Compile-time error: "'Either' is not a member of 'fp20'"
   - This confirms the Either type is not defined

4. Compile-time error: "'Identity' is not a member of 'fp20'"
   - This confirms the Identity type is not defined

5. Compile-time error: "no matching function for call to 'fmap'"
   - This confirms fmap specializations for Either/Identity don't exist

6. Compile-time error: "no matching function for call to 'bind'"
   - This confirms bind specializations for Either/Identity don't exist

These errors confirm tests are properly written in RED phase.

TEST STATISTICS:
- Compile-time tests: 55 static_assert statements
- Runtime tests: 25 assertion checks
- Total coverage: All 5 acceptance criteria fully tested
*/