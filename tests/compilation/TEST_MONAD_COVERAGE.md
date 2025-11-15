# Monad Concept Test Coverage Report
## Story FP-004a - TDD RED Phase

This document maps test cases to acceptance criteria, demonstrating complete coverage.

---

## Test Files Created

1. **Compile-time Tests**: `/Users/kdridi/Documents/fp++20/tests/compilation/test_monad_concept.cpp`
   - 487 lines of static_assert-based tests
   - Tests concept definitions, type constraints, and compile-time requirements

2. **Runtime Tests**: `/Users/kdridi/Documents/fp++20/tests/runtime/test_monad_runtime.cpp`
   - 536 lines of runtime assertion-based tests
   - Tests actual behavior of monadic operations

---

## Acceptance Criteria Coverage

### ✅ AC1: Concept Definition Compiles

**Requirement**:
- `static_assert(Monad<std::optional<int>>)` should pass (when implemented)
- `static_assert(!Monad<int>)` should pass (int is not a monad)

**Test Coverage**:
```cpp
// Compile-time tests (lines 45-122)
namespace test_monad_concept_definition {
    // Positive tests - types that SHOULD be Monads
    static_assert(fp20::concepts::Monad<std::optional<int>>);
    static_assert(fp20::concepts::Monad<std::optional<std::string>>);
    static_assert(fp20::concepts::Monad<std::optional<double>>);
    static_assert(fp20::concepts::Monad<std::vector<int>>);
    static_assert(fp20::concepts::Monad<std::vector<std::string>>);

    // Negative tests - types that should NOT be Monads
    static_assert(!fp20::concepts::Monad<int>);
    static_assert(!fp20::concepts::Monad<double>);
    static_assert(!fp20::concepts::Monad<std::string>);

    // Edge cases
    static_assert(fp20::concepts::Monad<std::optional<std::optional<int>>>);
    static_assert(fp20::concepts::Monad<std::vector<ComplexType>>);
}
```

**Test Count**: 15 static_asserts

---

### ✅ AC2: Proper Subsumption

**Requirement**:
- Every type that satisfies `Monad<T>` must also satisfy `Applicative<T>`
- `Monad` refines `Applicative` in the concept hierarchy

**Test Coverage**:
```cpp
// Compile-time tests (lines 129-175)
namespace test_monad_subsumption {
    // Direct subsumption: Monad -> Applicative
    static_assert(fp20::concepts::Monad<std::optional<int>>
                  implies fp20::concepts::Applicative<std::optional<int>>);

    // Transitive subsumption: Monad -> Functor
    static_assert(fp20::concepts::Monad<std::optional<int>>
                  implies fp20::concepts::Functor<std::optional<int>>);

    // Complete hierarchy verification
    template<typename M>
    concept MonadImpliesHierarchy =
        fp20::concepts::Monad<M>
        && fp20::concepts::Applicative<M>
        && fp20::concepts::Functor<M>;

    static_assert(MonadImpliesHierarchy<std::optional<int>>);
    static_assert(MonadImpliesHierarchy<std::vector<int>>);
}
```

**Test Count**: 8 static_asserts

---

### ✅ AC3: return_ Operation Enforced

**Requirement**:
- Signature: `A → M<A>` (lifts a value into monadic context)
- Must be defined for any type claiming to be a Monad

**Compile-time Test Coverage**:
```cpp
// Compile-time tests (lines 182-224)
namespace test_return_operation {
    // Function existence
    static_assert(requires {
        fp20::return_<std::optional>(42);
        fp20::return_<std::vector>(42);
    });

    // Type signature verification
    static_assert(std::is_same_v<
        decltype(fp20::return_<std::optional>(42)),
        std::optional<int>
    >);

    static_assert(std::is_same_v<
        decltype(fp20::return_<std::vector>(42)),
        std::vector<int>
    >);

    // Constexpr support
    constexpr auto test_constexpr_return() {
        constexpr auto result = fp20::return_<std::optional>(42);
        return result.value_or(0);
    }
    static_assert(test_constexpr_return() == 42);

    // Equivalence to pure
    static_assert(requires {
        { fp20::return_<std::optional>(42) }
            -> std::same_as<decltype(fp20::pure<std::optional>(42))>;
    });
}
```

**Runtime Test Coverage**:
```cpp
// Runtime tests (lines 49-111)
void test_return_optional() {
    // Creates Some with given value
    auto opt_int = fp20::return_<std::optional>(42);
    assert(opt_int.has_value());
    assert(opt_int.value() == 42);

    // Works with various types
    auto opt_str = fp20::return_<std::optional>(std::string("hello"));
    auto opt_double = fp20::return_<std::optional>(3.14);
    auto opt_point = fp20::return_<std::optional>(Point{10, 20});
}

void test_return_vector() {
    // Creates singleton vector
    auto vec_int = fp20::return_<std::vector>(42);
    assert(vec_int.size() == 1);
    assert(vec_int[0] == 42);

    // Equivalent to pure
    auto ret_vec = fp20::return_<std::vector>(42);
    auto pure_vec = fp20::pure<std::vector>(42);
    assert(ret_vec.size() == pure_vec.size());
}
```

**Test Count**:
- Compile-time: 12 static_asserts
- Runtime: 20 assertions across 2 test functions

---

### ✅ AC4: bind Operation Enforced

**Requirement**:
- Signature: `M<A> → (A → M<B>) → M<B>`
- Enables monadic composition

**Compile-time Test Coverage**:
```cpp
// Compile-time tests (lines 231-323)
namespace test_bind_operation {
    // Function existence
    static_assert(requires {
        fp20::bind(
            std::optional<int>{42},
            [](int x) { return std::optional<double>{x * 2.0}; }
        );
    });

    // Type signature verification
    static_assert(std::is_same_v<
        decltype(fp20::bind(
            std::optional<int>{42},
            [](int x) { return std::optional<double>{x * 2.0}; }
        )),
        std::optional<double>
    >);

    // Type transformations
    static_assert(std::is_same_v<
        decltype(fp20::bind(
            std::vector<int>{1, 2, 3},
            [](int x) { return std::vector<double>{x * 1.0, x * 2.0}; }
        )),
        std::vector<double>
    >);

    // Constexpr support
    constexpr auto test_constexpr_bind() {
        constexpr auto result = fp20::bind(
            std::optional<int>{21},
            [](int x) { return std::optional<int>{x * 2}; }
        );
        return result.value_or(0);
    }
    static_assert(test_constexpr_bind() == 42);

    // Various callable types
    static_assert(requires {
        fp20::bind(std::optional<int>{}, [](int x) { return std::optional<int>{x}; });
        fp20::bind(std::optional<int>{}, std::function<std::optional<int>(int)>{});
    });
}
```

**Runtime Test Coverage**:
```cpp
// Runtime tests (lines 118-312)

// Basic bind behavior for optional
void test_bind_optional_basic() {
    // Some >>= f returns Some
    auto result1 = fp20::bind(
        std::optional<int>{21},
        [](int x) { return std::optional<int>{x * 2}; }
    );
    assert(result1.value() == 42);

    // None >>= f returns None
    auto result2 = fp20::bind(
        std::optional<int>{},
        [](int x) { return std::optional<int>{x * 2}; }
    );
    assert(!result2.has_value());
}

// Type transformations
void test_bind_optional_type_transform() {
    // int -> string
    auto result1 = fp20::bind(
        std::optional<int>{42},
        [](int x) { return std::optional<std::string>{std::to_string(x)}; }
    );
    assert(result1.value() == "42");

    // Chained transformations
    auto result4 = fp20::bind(
        fp20::bind(
            std::optional<int>{21},
            [](int x) { return std::optional<int>{x * 2}; }
        ),
        [](int x) { return std::optional<std::string>{std::to_string(x)}; }
    );
    assert(result4.value() == "42");
}

// List monad (vector) bind
void test_bind_vector_basic() {
    // Replication
    auto result1 = fp20::bind(
        std::vector<int>{1, 2, 3},
        [](int x) { return std::vector<int>{x, x}; }
    );
    assert(result1.size() == 6);

    // Empty propagation
    auto result3 = fp20::bind(
        std::vector<int>{1, 2, 3},
        [](int x) { return std::vector<int>{}; }
    );
    assert(result3.empty());
}

// FlatMap behavior
void test_bind_vector_flatmap() {
    // Conditional expansion
    auto result = fp20::bind(
        std::vector<int>{1, 2, 3, 4, 5},
        [](int x) {
            return x % 2 == 0 ? std::vector<int>{x, x} : std::vector<int>{};
        }
    );
    assert(result.size() == 4);  // Only even numbers, duplicated
}
```

**Test Count**:
- Compile-time: 15 static_asserts
- Runtime: 45+ assertions across 6 test functions

---

### ✅ AC5: No Breaking Changes

**Requirement**:
- Existing Functor/Applicative tests still pass
- No existing code breaks

**Test Coverage**:
```cpp
// Compile-time tests (lines 330-378)
namespace test_no_breaking_changes {
    // Functor still works
    static_assert(fp20::concepts::Functor<std::optional<int>>);
    static_assert(fp20::concepts::Functor<std::vector<int>>);

    // Applicative still works
    static_assert(fp20::concepts::Applicative<std::optional<int>>);
    static_assert(fp20::concepts::Applicative<std::vector<int>>);

    // fmap still works
    static_assert(requires {
        fp20::fmap([](int x) { return x * 2; }, std::optional<int>{42});
        fp20::fmap([](int x) { return x * 2; }, std::vector<int>{1, 2, 3});
    });

    // pure still works
    static_assert(requires {
        fp20::pure<std::optional>(42);
        fp20::pure<std::vector>(42);
    });

    // apply still works
    static_assert(requires {
        fp20::apply(
            std::optional<std::function<int(int)>>{},
            std::optional<int>{42}
        );
    });
}
```

**Runtime Test Coverage**:
```cpp
// Runtime tests (lines 408-447)
void test_monad_applicative_integration() {
    // fmap still works on monads
    auto fmapped = fp20::fmap([](int x) { return x * 2; }, std::optional<int>{21});
    assert(fmapped.value() == 42);

    // pure creates same result as return_
    auto pure_opt = fp20::pure<std::optional>(42);
    auto return_opt = fp20::return_<std::optional>(42);
    assert(pure_opt.value() == return_opt.value());

    // apply still works on monads
    auto applied = fp20::apply(opt_func, opt_val);
    assert(applied.value() == 15);

    // Can mix bind and fmap
    auto result = fp20::bind(
        fp20::fmap([](int x) { return x * 2; }, std::optional<int>{10}),
        [](int x) { return std::optional<int>{x + 5}; }
    );
    assert(result.value() == 25);
}
```

**Test Count**:
- Compile-time: 10 static_asserts
- Runtime: 8 assertions

---

## Additional Test Coverage

### Edge Cases
```cpp
namespace test_monad_edge_cases {
    // Const value types
    static_assert(fp20::concepts::Monad<std::optional<const int>>);

    // Smart pointers
    static_assert(fp20::concepts::Monad<std::optional<std::unique_ptr<int>>>);

    // Structural rejection
    struct HasValueType { using value_type = int; };
    static_assert(!fp20::concepts::Monad<HasValueType>);
}
```

### Monad Laws (Informal Runtime Tests)
```cpp
// Left Identity: return a >>= f ≡ f a
void test_monad_law_left_identity() {
    auto f = [](int x) { return std::optional<int>{x * 2}; };
    auto left = fp20::bind(fp20::return_<std::optional>(21), f);
    auto right = f(21);
    assert(left.value() == right.value());
}

// Right Identity: m >>= return ≡ m
void test_monad_law_right_identity() {
    auto m = std::optional<int>{42};
    auto result = fp20::bind(m, [](int x) { return fp20::return_<std::optional>(x); });
    assert(result.value() == m.value());
}

// Associativity: (m >>= f) >>= g ≡ m >>= (λx -> f x >>= g)
void test_monad_law_associativity() {
    auto m = std::optional<int>{10};
    auto f = [](int x) { return std::optional<int>{x + 5}; };
    auto g = [](int x) { return std::optional<int>{x * 2}; };

    auto left = fp20::bind(fp20::bind(m, f), g);
    auto right = fp20::bind(m, [f, g](int x) { return fp20::bind(f(x), g); });

    assert(left.value() == right.value());
}
```

### Custom Monad Extensibility
```cpp
namespace test_custom_monad {
    template<typename T>
    class Maybe { /* ... */ };

    template<typename T>
    struct fp20::concepts::is_monad_opt_in<Maybe<T>> : std::true_type {};

    static_assert(fp20::concepts::Monad<Maybe<int>>);
}
```

---

## Test Statistics

### Compile-time Tests (`test_monad_concept.cpp`)
- **Total Lines**: 487
- **Static Asserts**: 60+
- **Test Namespaces**: 7
- **Coverage Areas**:
  - Concept definition: 15 tests
  - Subsumption: 8 tests
  - return_ operation: 12 tests
  - bind operation: 15 tests
  - Breaking changes: 10 tests
  - Edge cases: 5+ tests
  - Custom monads: 2 tests

### Runtime Tests (`test_monad_runtime.cpp`)
- **Total Lines**: 536
- **Assert Statements**: 100+
- **Test Functions**: 13
- **Coverage Areas**:
  - return_ for optional: 12 assertions
  - return_ for vector: 8 assertions
  - bind for optional (basic): 12 assertions
  - bind for optional (transforms): 16 assertions
  - bind for optional (chaining): 12 assertions
  - bind for vector (basic): 12 assertions
  - bind for vector (flatMap): 15 assertions
  - bind for vector (chaining): 8 assertions
  - Monad laws: 18 assertions
  - Integration: 8 assertions

---

## Expected Failures (RED Phase Verification)

### Compile-time Failure
```bash
$ g++ -std=c++20 -I./include -c tests/compilation/test_monad_concept.cpp

tests/compilation/test_monad_concept.cpp:38:10: fatal error:
    'fp20/concepts/monad.hpp' file not found
   38 | #include <fp20/concepts/monad.hpp>
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~
1 error generated.
```
✅ **VERIFIED**: Test fails as expected

### Subsequent Expected Errors (once header exists)
1. `'Monad' is not a member of 'fp20::concepts'` - concept not defined
2. `'return_' is not a member of 'fp20'` - function not implemented
3. `'bind' is not a member of 'fp20'` - function not implemented
4. `'is_monad_opt_in' is not a member of 'fp20::concepts'` - trait not defined

### Runtime Failure (once compiled)
```bash
Linker errors:
- Undefined reference to 'fp20::return_<std::optional>'
- Undefined reference to 'fp20::return_<std::vector>'
- Undefined reference to 'fp20::bind'
```

---

## Conclusion

✅ **All 5 Acceptance Criteria have comprehensive test coverage**
✅ **Tests fail as expected in RED phase**
✅ **Tests are structured to guide GREEN phase implementation**
✅ **Test count exceeds requirements for thorough validation**
✅ **Both compile-time and runtime aspects are tested**

**Total Test Coverage**:
- **160+ individual test assertions**
- **13 test functions (runtime)**
- **7 test namespaces (compile-time)**
- **1000+ lines of test code**

**Ready for TDD-Enforcer Validation**: ✅
