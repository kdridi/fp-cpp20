# FP++20 Project Structure

This document explains the organization of the FP++20 library codebase.

## Directory Layout

```
fp++20/
├── .vscode/                    # VSCode configuration
│   ├── settings.json           # Editor settings, IntelliSense config
│   ├── c_cpp_properties.json   # C++ IntelliSense paths
│   └── tasks.json              # Build and test tasks
├── include/                    # Public header files
│   └── fp20/
│       ├── fp20.hpp           # Single include file (convenience)
│       ├── concepts/          # Type classes and concept definitions
│       │   ├── functor.hpp            # Functor concept
│       │   ├── applicative.hpp        # Applicative concept
│       │   ├── monad.hpp              # Monad concept
│       │   ├── monoid.hpp             # Monoid concept
│       │   ├── either_identity_concepts.hpp   # Opt-ins for Either/Identity
│       │   ├── state_concepts.hpp             # Opt-ins for State
│       │   ├── reader_concepts.hpp            # Opt-ins for Reader
│       │   ├── writer_concepts.hpp            # Opt-ins for Writer
│       │   ├── io_concepts.hpp                # Opt-ins for IO
│       │   └── list_concepts.hpp              # Opt-ins for List
│       └── monads/            # Monad implementations
│           ├── identity.hpp           # Identity monad
│           ├── either.hpp             # Either monad (error handling)
│           ├── state.hpp              # State monad
│           ├── reader.hpp             # Reader monad (dependency injection)
│           ├── writer.hpp             # Writer monad (logging)
│           ├── io.hpp                 # IO monad (side effects)
│           └── list.hpp               # List monad (non-determinism)
├── tests/                      # Test suites
│   ├── compilation/           # Compile-time tests
│   │   ├── test_concepts.cpp
│   │   ├── test_either_identity.cpp
│   │   ├── test_state_monad.cpp
│   │   ├── test_reader_monad.cpp
│   │   ├── test_writer_monad.cpp
│   │   ├── test_io_monad.cpp
│   │   └── test_list_monad.cpp
│   ├── unit/                  # Unit tests (Catch2)
│   │   ├── test_functor_concept.cpp
│   │   ├── test_functor_laws.cpp
│   │   ├── test_applicative_concept.cpp
│   │   └── test_applicative_laws.cpp
│   ├── properties/            # Property-based tests
│   └── runtime/               # Runtime behavior tests
├── docs/                       # Documentation
├── examples/                   # Example code
├── scripts/                    # Utility scripts
├── build/                      # Build output (gitignored)
├── CMakeLists.txt             # CMake configuration
└── README.md                  # Project overview
```

## Include Path Conventions

### For Library Users

**Option 1: Single include (recommended for simplicity)**
```cpp
#include <fp20/fp20.hpp>  // Includes everything
```

**Option 2: Selective includes (recommended for compile time)**
```cpp
#include <fp20/concepts/monad.hpp>      // Just the Monad concept
#include <fp20/monads/either.hpp>       // Just the Either monad
#include <fp20/concepts/either_identity_concepts.hpp>  // Concept opt-ins
```

### For Library Developers

When working inside the library:
- **Monad implementations**: `#include <fp20/monads/X.hpp>`
- **Concept definitions**: `#include <fp20/concepts/X.hpp>`
- **Concept opt-ins**: `#include <fp20/concepts/X_concepts.hpp>`

## File Naming Conventions

- **Monad implementations**: `<monad_name>.hpp` (e.g., `either.hpp`, `state.hpp`)
- **Concept definitions**: `<concept_name>.hpp` (e.g., `functor.hpp`, `monad.hpp`)
- **Concept opt-ins**: `<monad_name>_concepts.hpp` (e.g., `state_concepts.hpp`)
- **Tests**: `test_<feature>_<aspect>.cpp` (e.g., `test_state_monad.cpp`)

## Build System

### CMake Targets
- `compilation_tests` - Compile-time tests
- `unit_tests` - Runtime unit tests (Catch2)

### Building
```bash
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
```

### Testing
```bash
ctest --test-dir build --output-on-failure
```

### VSCode Integration
- Use `Cmd+Shift+B` to build
- Use `Cmd+Shift+T` to run tests
- IntelliSense configured with proper include paths
- `compile_commands.json` generated for accurate code intelligence

## Code Organization Philosophy

### Separation of Concerns
- **Concepts** define type class interfaces (what operations a type must support)
- **Monads** provide concrete implementations
- **Concept opt-ins** connect implementations to concepts (explicit trait specialization)

### Why This Structure?

1. **Clear separation**: Type classes vs implementations vs opt-ins
2. **Easy navigation**: All monads in one place, all concepts in another
3. **Compile-time efficiency**: Include only what you need
4. **Maintenance**: Changes to one monad don't affect others
5. **Scalability**: Easy to add new monads or concepts

## Adding a New Monad

To add a new monad (e.g., `Maybe`):

1. Create implementation: `include/fp20/monads/maybe.hpp`
2. Create opt-ins: `include/fp20/concepts/maybe_concepts.hpp`
3. Add includes to `include/fp20/fp20.hpp`
4. Create tests: `tests/compilation/test_maybe_monad.cpp`
5. Update `CMakeLists.txt` to include new test file

## VSCode Setup

The repository includes VSCode configuration for optimal C++ development:

- **IntelliSense**: Automatically configured with correct include paths
- **Compile Commands**: Uses `build/compile_commands.json` for accurate code analysis
- **Tasks**: Pre-configured build and test tasks
- **C++20**: Full C++20 standard support enabled

No additional setup needed - just open the folder in VSCode!
