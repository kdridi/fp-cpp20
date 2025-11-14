# TDD Roadmap for FP++20

## Core TDD Principle: RED → GREEN → REFACTOR

Every single feature follows this cycle:
1. **RED**: Write a test that fails
2. **GREEN**: Write minimal code to pass
3. **REFACTOR**: Improve code while keeping tests green

## Project Roadmap with TDD Focus

### EPIC 1: FUNCTOR FOUNDATION
**Goal**: Implement the Functor abstraction with full law compliance
**TDD Approach**: Each story starts with failing tests

#### Sprint 1.1: Functor Concept Definition
**User Stories** (in TDD order):

1. **Story: Define Functor Concept** [Effort: S]
   - RED: Write test checking `fp20::Functor<T>` concept exists
   - RED: Test that std::vector satisfies Functor
   - RED: Test that std::optional satisfies Functor
   - GREEN: Implement minimal concept in `include/fp20/concepts/functor.hpp`
   - REFACTOR: Improve concept constraints

2. **Story: Implement fmap for std::vector** [Effort: M]
   - RED: Write test for `fmap(f, vector)` syntax
   - RED: Test type transformation (int → string)
   - GREEN: Implement fmap specialization for vector
   - REFACTOR: Optimize implementation

3. **Story: Implement fmap for std::optional** [Effort: S]
   - RED: Write test for Some case
   - RED: Write test for None case
   - GREEN: Implement fmap specialization
   - REFACTOR: Ensure move semantics

4. **Story: Verify Functor Laws** [Effort: M]
   - RED: Write identity law tests
   - RED: Write composition law tests
   - GREEN: Ensure implementations satisfy laws
   - REFACTOR: Add compile-time law checking

5. **Story: Create Custom Functor (Maybe)** [Effort: L]
   - RED: Write tests for Maybe<T> type
   - RED: Test Maybe as Functor instance
   - GREEN: Implement Maybe with fmap
   - REFACTOR: Add pattern matching support

### EPIC 2: APPLICATIVE ABSTRACTION
**Goal**: Build Applicative on top of Functor
**TDD Approach**: Test-first for operators and laws

#### Sprint 2.1: Applicative Core
**User Stories**:

1. **Story: Define Applicative Concept** [Effort: M]
   - RED: Test `fp20::Applicative<T>` requires `fp20::Functor<T>`
   - RED: Test pure function exists
   - RED: Test apply (<*>) operator exists
   - GREEN: Implement concept and operations

2. **Story: Implement Applicative for std::optional** [Effort: M]
   - RED: Test `pure(value)` creates optional
   - RED: Test `apply` for function in optional
   - GREEN: Implement specializations

3. **Story: Implement liftA2** [Effort: S]
   - RED: Test lifting binary functions
   - GREEN: Implement generic liftA2
   - REFACTOR: Add liftA3, liftA4

4. **Story: Verify Applicative Laws** [Effort: L]
   - RED: Identity law tests
   - RED: Composition law tests
   - RED: Homomorphism law tests
   - RED: Interchange law tests
   - GREEN: Ensure compliance

### EPIC 3: MONAD IMPLEMENTATION
**Goal**: Complete the hierarchy with Monad
**TDD Approach**: Complex state handling via tests

#### Sprint 3.1: Monad Essentials
**User Stories**:

1. **Story: Define Monad Concept** [Effort: M]
   - RED: Test Monad requires Applicative
   - RED: Test bind (>>=) operation exists
   - RED: Test return operation
   - GREEN: Implement concept

2. **Story: Implement Monad for std::optional** [Effort: M]
   - RED: Test chaining with bind
   - RED: Test early termination on None
   - GREEN: Implement bind operation

3. **Story: Create Either Monad** [Effort: L]
   - RED: Test Left/Right construction
   - RED: Test error propagation
   - RED: Test successful chaining
   - GREEN: Full Either implementation

4. **Story: Implement do-notation equivalent** [Effort: XL]
   - RED: Test monadic comprehension
   - GREEN: Macro or template solution
   - REFACTOR: Optimize compile times

5. **Story: Verify Monad Laws** [Effort: L]
   - RED: Left identity tests
   - RED: Right identity tests
   - RED: Associativity tests
   - GREEN: Ensure compliance

## Coordination with Agent Team

### 1. tdd-enforcer Agent
- **Role**: Ensures every commit has tests written first
- **Integration Points**:
  - Pre-commit hook validation
  - CI/CD pipeline checks
  - Test coverage reporting

### 2. cpp20-unit-test-writer Agent
- **Role**: Generates comprehensive test cases
- **Collaboration**:
  - Generate property-based tests
  - Create edge case scenarios
  - Produce law verification tests

### 3. cpp20-expert Agent
- **Role**: Provides implementation guidance
- **Consultation Areas**:
  - Concept definition optimization
  - Template metaprogramming patterns
  - Compile-time computation strategies

## Test Organization Structure

```
tests/
├── unit/                    # TDD unit tests (written FIRST)
│   ├── test_functor_concept.cpp
│   ├── test_functor_instances.cpp
│   ├── test_applicative_concept.cpp
│   ├── test_applicative_instances.cpp
│   ├── test_monad_concept.cpp
│   └── test_monad_instances.cpp
├── properties/              # Property-based testing
│   ├── test_functor_properties.cpp
│   ├── test_applicative_properties.cpp
│   └── test_monad_properties.cpp
└── compilation/            # Compile-time tests
    ├── test_compile_time_functor.cpp
    ├── test_compile_time_applicative.cpp
    └── test_compile_time_monad.cpp
```

## TDD Enforcement Mechanisms

1. **Git Hooks** (pre-commit):
   ```bash
   # Check test files exist before implementation files
   # Verify test files were modified before source files
   # Ensure all tests pass before allowing commit
   ```

2. **CMake Integration**:
   ```cmake
   # FP20_STRICT_TDD option enforces:
   # - Test targets must be defined before library code
   # - Coverage must meet minimum threshold
   # - All tests must pass for successful build
   ```

3. **CI Pipeline**:
   - Test execution before build
   - Coverage reporting (must be > 95%)
   - Mutation testing to verify test quality

## Success Metrics

- **Test Coverage**: > 95% line and branch coverage
- **Test-First Compliance**: 100% of features have tests written first
- **Law Compliance**: All type classes pass mathematical law tests
- **Compile-Time Safety**: Zero runtime errors possible for valid usage
- **Documentation Coverage**: Every public API has example tests

## Next Immediate Actions (TDD Order)

1. ✅ Project structure created
2. ✅ CMake configuration with testing
3. ✅ First failing test for Functor concept written
4. ⏳ **NEXT**: Implement minimal Functor concept to make test pass (GREEN phase)
5. ⏳ Write failing test for fmap on std::vector
6. ⏳ Implement fmap for std::vector
7. ⏳ Continue RED-GREEN-REFACTOR cycle

## Command Reference

```bash
# Run tests (will fail initially - that's good!)
cmake -B build -S . -DCMAKE_CXX_STANDARD=20
cmake --build build
ctest --test-dir build --output-on-failure

# Run with coverage
cmake -B build -S . -DFP20_ENABLE_COVERAGE=ON
cmake --build build
ctest --test-dir build
gcovr -r . --html --html-details -o coverage.html

# Strict TDD mode (default ON)
cmake -B build -S . -DFP20_STRICT_TDD=ON
```