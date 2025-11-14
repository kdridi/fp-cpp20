# fp++20 - Functional Programming in Modern C++20

A header-only C++20 library implementing Haskell-inspired functional programming concepts: **Functor**, **Applicative**, and **Monad**.

## Philosophy

This library is developed using **strict Test-Driven Development (TDD)**:
- **RED**: Write failing tests first
- **GREEN**: Implement minimal code to pass tests
- **REFACTOR**: Improve design while keeping tests green

Every line of production code is justified by a test written beforehand.

## Features (Roadmap)

### EPIC 1: Functor
- [ ] Functor concept definition
- [ ] `fmap` for standard containers (`std::vector`, `std::optional`)
- [ ] Functor law verification (identity, composition)
- [ ] Custom Functor types

### EPIC 2: Applicative
- [ ] Applicative concept (extends Functor)
- [ ] `pure` and `<*>` (apply) operations
- [ ] Applicative laws verification
- [ ] Applicative instances for standard types

### EPIC 3: Monad
- [ ] Monad concept (extends Applicative)
- [ ] `return` (pure) and `>>=` (bind) operations
- [ ] Monad law verification
- [ ] Do-notation style syntax (C++20 coroutines?)

## Project Structure

```
fp++20/
├── include/fp20/          # Header-only library
│   ├── concepts/          # Core type class concepts
│   ├── functors/          # Functor implementations
│   ├── applicatives/      # Applicative implementations
│   └── monads/            # Monad implementations
├── tests/
│   ├── unit/              # Runtime tests (TDD)
│   ├── compilation/       # Compile-time tests (static_assert)
│   └── properties/        # Property-based tests
├── examples/              # Usage examples
└── docs/                  # Documentation
```

## Building & Testing

### Requirements
- C++20 compliant compiler (GCC 10+, Clang 10+, MSVC 2019+)
- CMake 3.20+

### Build
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Run Tests (TDD Workflow)
```bash
# Run all tests
ctest --output-on-failure

# Or use custom target
cmake --build . --target test_all
```

## TDD Workflow

1. **Write failing test** (`tests/unit/` or `tests/compilation/`)
2. **Verify test fails** (compilation error or assertion)
3. **Implement minimal code** (`include/fp20/`)
4. **Verify test passes**
5. **Refactor** while keeping tests green
6. **Commit** with test + implementation together

## Usage Example (Future)

```cpp
#include <fp20/functor.hpp>
#include <vector>

int main() {
    std::vector<int> v = {1, 2, 3};
    auto doubled = fmap([](int x) { return x * 2; }, v);
    // doubled == {2, 4, 6}
}
```

## Contributing

This project follows strict TDD. All contributions must:
1. Include tests written BEFORE implementation
2. Pass all existing tests
3. Follow C++20 best practices
4. Include documentation

## License

TBD

## References

- Haskell Type Classes: https://www.haskell.org/tutorial/classes.html
- Category Theory for Programmers: https://bartoszmilewski.com/2014/10/28/category-theory-for-programmers-the-preface/
- C++20 Concepts: https://en.cppreference.com/w/cpp/language/constraints
