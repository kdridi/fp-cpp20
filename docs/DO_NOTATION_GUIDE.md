# FP++20 Do-Notation Guide

## Table of Contents

1. [Introduction](#introduction)
2. [Motivation](#motivation)
3. [Quick Start](#quick-start)
4. [Syntax Options](#syntax-options)
5. [Complete Examples](#complete-examples)
6. [Advanced Patterns](#advanced-patterns)
7. [Performance](#performance)
8. [Troubleshooting](#troubleshooting)
9. [API Reference](#api-reference)

---

## Introduction

Do-notation brings Haskell's elegant monadic syntax to C++20. It transforms verbose lambda chains into clean, imperative-style code while maintaining type safety and zero runtime overhead.

### What Problem Does This Solve?

**Before (Verbose):**
```cpp
auto result = bind(getX(), [](auto x) {
    return bind(getY(), [x](auto y) {
        return bind(getZ(), [x, y](auto z) {
            return pure(x + y + z);
        });
    });
});
```

**After (Elegant):**
```cpp
auto result = DO(
    x, getX(),
    y, getY(),
    z, getZ(),
    RETURN(x + y + z)
);
```

---

## Motivation

### Callback Hell in Monadic Code

Traditional monadic composition in C++ suffers from:
- **Deep nesting**: Each bind adds another level of indentation
- **Capture complexity**: Manual lambda capture lists ([x], [x,y], [x,y,z])
- **Readability**: Hard to follow the logical flow
- **Maintenance**: Adding/removing steps requires restructuring

### The Haskell Inspiration

Haskell's do-notation solved this 30 years ago:

```haskell
result = do
    x <- getX
    y <- getY
    z <- getZ
    return (x + y + z)
```

Our C++20 implementation brings this elegance while preserving:
- ✅ **Type safety** (compile-time type checking)
- ✅ **Zero overhead** (inlines to same assembly as manual bind)
- ✅ **Composability** (works with all monads)
- ✅ **Flexibility** (multiple syntax styles)

---

## Quick Start

### Installation

Include the header:
```cpp
#include <fp20/do_notation.hpp>
```

### Your First Do-Block

```cpp
#include <fp20/do_notation.hpp>
#include <optional>

int main() {
    auto result = DO(
        x, std::optional{10},
        y, std::optional{20},
        RETURN(std::optional{x + y})
    );

    std::cout << *result; // 30
}
```

### How It Works

The `DO(...)` macro expands to:
```cpp
bind(std::optional{10}, [](auto x) {
    return bind(std::optional{20}, [x](auto y) {
        return std::optional{x + y};
    });
});
```

But you write the elegant version!

---

## Syntax Options

FP++20 provides **three syntaxes** for different preferences:

### Option 1: DO(...) - Compact (Recommended)

**Best for:** Most use cases, maximum readability

```cpp
auto result = DO(
    username, getUsername(),
    password, getPassword(),
    token, authenticate(username, password),
    RETURN(token)
);
```

**Pros:**
- Most compact
- Clear variable bindings
- Easy to read

**Cons:**
- Commas in expressions need parentheses: `DO(x, foo(a, b), ...)` → `DO(x, (foo(a, b)), ...)`

### Option 2: MBEGIN/MEND - Imperative (GCC/Clang)

**Best for:** Developers who prefer C-style syntax

```cpp
auto result = MBEGIN
    MLET(username) = getUsername();
    MLET(password) = getPassword();
    MLET(token) = authenticate(username, password);
    MRETURN(token);
MEND
```

**Pros:**
- Looks like normal C++ code
- Semicolons feel natural
- Clear block structure

**Cons:**
- Requires GCC/Clang (uses statement expressions)
- Not available on MSVC

### Option 3: MChain - Explicit (Most Haskell-like)

**Best for:** Functional programmers, explicit control

```cpp
auto result =
    getUsername() >>= MLAMBDA(username,
    getPassword() >>= MLAMBDA(password,
    authenticate(username, password) >>= MLAMBDA(token,
    MPURE(token)
    )));
```

**Pros:**
- Most explicit
- Looks like Haskell >>= operator
- No macro magic

**Cons:**
- Most verbose
- Parenthesis matching required

---

## Complete Examples

### Example 1: Error Handling with Either

```cpp
#include <fp20/do_notation.hpp>
#include <fp20/monads/either.hpp>

using Result = Either<std::string, int>;

auto divide(int a, int b) -> Result {
    if (b == 0) return Result::left("Division by zero");
    return Result::right(a / b);
}

auto validate_positive(int x) -> Result {
    if (x <= 0) return Result::left("Must be positive");
    return Result::right(x);
}

// Compose with automatic error propagation
auto computation = DO(
    x, Result::right(100),
    y, divide(x, 10),        // y = 10
    z, validate_positive(y), // OK: 10 > 0
    RETURN(Result::right(z * 2))
);

// computation == Right(20)
```

**What happens on error:**
```cpp
auto failed = DO(
    x, Result::right(100),
    y, divide(x, 0),         // ERROR: Division by zero
    z, validate_positive(y), // Never executed
    RETURN(Result::right(z))
);

// failed == Left("Division by zero")
```

### Example 2: State Threading

```cpp
#include <fp20/do_notation.hpp>
#include <fp20/monads/state.hpp>

// Game state management
struct GameState {
    int score;
    int lives;
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

auto level_complete = DO(
    _, add_points(100),        // +100 points
    _, add_points(50),         // +50 bonus
    _, lose_life(),            // -1 life
    final, get<GameState>(),
    RETURN(pure<State, GameState>(final))
);

// Run with initial state
GameState initial{0, 3};
auto [result, _] = level_complete.runState(initial);
// result.score == 150
// result.lives == 2
```

### Example 3: IO Composition

```cpp
#include <fp20/do_notation.hpp>
#include <fp20/monads/io.hpp>

// Build complex IO programs
auto greet_user = DO(
    _, putStr("Enter your name: "),
    name, getLine(),
    _, putStrLn("Hello, " + name + "!"),
    _, putStrLn("Welcome to FP++20!"),
    RETURN(pure<IO>(Unit{}))
);

// Nothing happens yet - just a description
// Execute at the edge:
std::move(greet_user).unsafeRun();
```

**Interactive example:**
```cpp
auto calculator = DO(
    _, putStrLn("Simple Calculator"),
    _, putStr("First number: "),
    a_str, getLine(),
    _, putStr("Second number: "),
    b_str, getLine(),
    a, pure<IO>(std::stoi(a_str)),
    b, pure<IO>(std::stoi(b_str)),
    sum, pure<IO>(a + b),
    _, putStrLn("Result: " + std::to_string(sum)),
    RETURN(pure<IO>(sum))
);

// Execute the program
auto result = std::move(calculator).unsafeRun();
```

### Example 4: Reader-based Dependency Injection

```cpp
#include <fp20/do_notation.hpp>
#include <fp20/monads/reader.hpp>

struct AppConfig {
    std::string api_url;
    std::string api_key;
    int timeout;
};

auto build_request = DO(
    url, asks<AppConfig>([](auto& c) { return c.api_url; }),
    key, asks<AppConfig>([](auto& c) { return c.api_key; }),
    timeout, asks<AppConfig>([](auto& c) { return c.timeout; }),
    RETURN(pure<AppConfig>(
        "GET " + url + " (key=" + key +
        ", timeout=" + std::to_string(timeout) + ")"
    ))
);

// Inject configuration
AppConfig config{
    "https://api.example.com/users",
    "secret-key-123",
    30
};

auto request = build_request.runReader(config);
// "GET https://api.example.com/users (key=secret-key-123, timeout=30)"
```

### Example 5: Optional Chaining

```cpp
#include <fp20/do_notation.hpp>
#include <optional>

struct User {
    std::string name;
    std::optional<int> account_id;
};

struct Account {
    int id;
    std::optional<double> balance;
};

auto find_user(int id) -> std::optional<User>;
auto find_account(int id) -> std::optional<Account>;

// Chain nullable operations
auto get_balance = DO(
    user, find_user(123),
    acc_id, user.account_id,
    account, find_account(acc_id),
    balance, account.balance,
    RETURN(std::optional{balance})
);

// Short-circuits on first None
// No null checks needed!
```

### Example 6: List Comprehensions

```cpp
#include <fp20/do_notation.hpp>
#include <vector>

// Pythagorean triples
std::vector<int> range(int start, int end) {
    std::vector<int> result;
    for (int i = start; i <= end; ++i) {
        result.push_back(i);
    }
    return result;
}

auto pythagorean_triples = DO(
    x, range(1, 20),
    y, range(x, 20),
    z, range(y, 20),
    RETURN([x, y, z]() -> std::vector<std::tuple<int, int, int>> {
        if (x*x + y*y == z*z) {
            return {std::make_tuple(x, y, z)};
        }
        return {};
    }())
);

// Result: [(3,4,5), (5,12,13), (6,8,10), (8,15,17), (9,12,15), (12,16,20)]
```

**Cartesian product:**
```cpp
auto pairs = DO(
    x, std::vector{1, 2, 3},
    y, std::vector{10, 20},
    RETURN(std::vector{std::make_pair(x, y)})
);

// Result: [(1,10), (1,20), (2,10), (2,20), (3,10), (3,20)]
```

---

## Advanced Patterns

### Pattern 1: Nested Do-Blocks

```cpp
auto outer = DO(
    x, getX(),
    y, DO(
        a, getA(),
        b, getB(),
        RETURN(a + b)
    ),
    RETURN(x * y)
);
```

### Pattern 2: Ignoring Results

Use `_` for values you don't need:

```cpp
auto program = DO(
    _, initialize(),        // Run but ignore result
    _, setup_database(),    // Run but ignore result
    result, main_logic(),
    RETURN(result)
);
```

### Pattern 3: Conditional Execution

```cpp
auto conditional = DO(
    user, get_user(),
    _, (user.is_admin
        ? log_admin_access()
        : log_user_access()),
    data, fetch_data(user),
    RETURN(data)
);
```

### Pattern 4: Exception-Safe Cleanup

```cpp
auto with_resource = DO(
    resource, acquire_resource(),
    result, use_resource(resource),
    _, release_resource(resource),
    RETURN(result)
);
```

### Pattern 5: Multiple Error Types

```cpp
using Error1 = Either<std::string, int>;
using Error2 = Either<std::string, std::string>;

auto convert_error = [](int x) -> Error2 {
    return Error2::right(std::to_string(x));
};

auto composed = DO(
    x, Error1::right(42),
    y, convert_error(x),
    RETURN(Error2::right("Value: " + y))
);
```

---

## Performance

### Zero Runtime Overhead

The macros expand to inline lambda chains. With optimization enabled, the compiler generates **identical assembly** to hand-written code.

**Benchmark:**
```cpp
// Do-notation version
auto do_version = DO(
    x, compute1(),
    y, compute2(x),
    z, compute3(y),
    RETURN(z)
);

// Manual version
auto manual_version = bind(compute1(), [](auto x) {
    return bind(compute2(x), [](auto y) {
        return compute3(y);
    });
});

// GCC -O3: Same assembly!
// Clang -O2: Same assembly!
```

### Compile-Time Characteristics

- **Template depth:** O(n) where n = number of bindings
- **Compile time:** Comparable to manual bind chains
- **Binary size:** No increase (macros inline away)

**Tips for fast compilation:**
```cpp
// If you hit template depth limits:
// Add to your build flags:
-ftemplate-depth=512
```

### Memory Usage

- **Stack usage:** Identical to manual bind
- **Heap allocations:** Only what the monad itself requires
- **Move semantics:** Fully preserved

---

## Troubleshooting

### Common Issues

#### Issue 1: Commas in Expressions

**Error:**
```cpp
DO(x, foo(a, b), ...)  // WRONG: Comma confuses macro
```

**Solution:**
```cpp
DO(x, (foo(a, b)), ...)  // CORRECT: Parenthesize
```

#### Issue 2: Template Syntax

**Error:**
```cpp
DO(x, get<int>(), ...)  // WRONG: < confuses preprocessor
```

**Solution:**
```cpp
DO(x, (get<int>()), ...)  // CORRECT: Parenthesize
```

#### Issue 3: Variable Name Conflicts

**Error:**
```cpp
DO(x, m1, x, m2, ...)  // WRONG: x used twice
```

**Solution:**
```cpp
DO(x, m1, y, m2, ...)  // CORRECT: Unique names
```

#### Issue 4: Compilation Errors

**Cryptic error messages:**
```
error: no matching function for call to 'bind'
```

**Debug steps:**
1. Check each monad is actually a Monad (has bind, pure)
2. Verify return types match expectations
3. Ensure RETURN(...) wraps in correct monad type
4. Use `static_assert` to inspect types:
   ```cpp
   auto x = getX();
   static_assert(concepts::Monad<decltype(x)>);
   ```

### Debugging Tips

#### Expand Macros

```bash
# GCC
g++ -E -P your_file.cpp > expanded.cpp

# Clang
clang++ -E -P your_file.cpp > expanded.cpp
```

This shows what the macro expands to.

#### Use Compiler Explorer

https://godbolt.org/ - Paste your code to see:
- Assembly output
- Macro expansions
- Optimization effects

#### Type Introspection

```cpp
#include <boost/type_index.hpp>

auto result = DO(...);
std::cout << boost::typeindex::type_id_with_cvr<decltype(result)>().pretty_name();
```

---

## API Reference

### Core Macros

#### `DO(var1, monad1, var2, monad2, ..., RETURN(expr))`

Compact comma-separated do-notation.

**Parameters:**
- `varN`: Variable name for binding
- `monadN`: Monadic expression to bind
- `expr`: Final expression to return

**Returns:** Result monad containing `expr`

**Example:**
```cpp
DO(x, getX(), y, getY(), RETURN(x + y))
```

---

#### `RETURN(expr)`

Terminates a DO block with a pure value.

**Parameters:**
- `expr`: Expression to lift into monad

**Returns:** The expression (wrapped by surrounding monad)

**Example:**
```cpp
RETURN(42)
RETURN(x + y)
RETURN(std::make_tuple(a, b, c))
```

---

#### `MLAMBDA(var, body)`

Creates a lambda for manual bind chains.

**Parameters:**
- `var`: Variable name
- `body`: Lambda body

**Returns:** Lambda `[](auto var) { return body; }`

**Example:**
```cpp
bind(getX(), MLAMBDA(x, RETURN(x * 2)))
```

---

### Helper Macros

#### `MPURE(expr)`

Explicit pure/return for type inference.

**Example:**
```cpp
MPURE(42)  // Let compiler infer monad type
```

---

#### `YIELD(expr)`

Alias for RETURN (generator-like feel).

**Example:**
```cpp
DO(x, getX(), YIELD(x))
```

---

#### `MIGNORE(monad)`

Bind and discard result.

**Example:**
```cpp
MIGNORE(initialize())
```

---

### Type-Specific Constructors

```cpp
PURE_OPTIONAL(x)       // std::optional{x}
PURE_VECTOR(x)         // std::vector{x}
PURE_IO(x)             // fp20::pure<IO>(x)
PURE_STATE(x)          // fp20::pure(x)
PURE_READER(x)         // fp20::pure(x)
PURE_EITHER_LEFT(e)    // Either::left(e)
PURE_EITHER_RIGHT(v)   // Either::right(v)
```

---

## Best Practices

### 1. One Binding Per Line

**Bad:**
```cpp
DO(x, getX(), y, getY(), z, getZ(), RETURN(x+y+z))
```

**Good:**
```cpp
DO(
    x, getX(),
    y, getY(),
    z, getZ(),
    RETURN(x + y + z)
)
```

### 2. Use Meaningful Names

**Bad:**
```cpp
DO(a, f1(), b, f2(), c, f3(), RETURN(a+b+c))
```

**Good:**
```cpp
DO(
    user, fetch_user(),
    posts, fetch_posts(user),
    comments, fetch_comments(posts),
    RETURN(render(user, posts, comments))
)
```

### 3. Limit Nesting Depth

**Bad:**
```cpp
DO(
    x, DO(y, DO(z, DO(...))))
)
```

**Good:**
```cpp
auto inner = DO(z, getZ(), RETURN(z));
auto middle = DO(y, getY(), result, inner, RETURN(result));
auto outer = DO(x, getX(), result, middle, RETURN(result));
```

### 4. Use Type Aliases

**Bad:**
```cpp
DO(x, Either<std::string, int>::right(42), ...)
```

**Good:**
```cpp
using Result = Either<std::string, int>;
DO(x, Result::right(42), ...)
```

### 5. Comment Complex Logic

```cpp
auto complex = DO(
    // Fetch user from database
    user, find_user(id),

    // Validate permissions
    perms, check_permissions(user),

    // Load data if authorized
    data, load_data(user, perms),

    RETURN(data)
);
```

---

## Migration Guide

### From Manual Bind Chains

**Before:**
```cpp
bind(step1(), [](auto x) {
    return bind(step2(x), [x](auto y) {
        return bind(step3(x, y), [x, y](auto z) {
            return pure(x + y + z);
        });
    });
});
```

**After:**
```cpp
DO(
    x, step1(),
    y, step2(x),
    z, step3(x, y),
    RETURN(x + y + z)
)
```

**Steps:**
1. Identify bind chains
2. Extract variables from lambdas
3. Convert to DO syntax
4. Remove manual captures

### From Nested Callbacks

**Before:**
```cpp
async_op1([](auto x) {
    async_op2(x, [x](auto y) {
        async_op3(x, y, [x, y](auto z) {
            callback(x + y + z);
        });
    });
});
```

**After (with IO monad):**
```cpp
auto program = DO(
    x, async_op1(),
    y, async_op2(x),
    z, async_op3(x, y),
    RETURN(x + y + z)
);

run_async(program, callback);
```

---

## FAQ

### Q: Does this work with custom monads?

**A:** Yes! Any type with `bind` and `pure` works automatically.

```cpp
class MyMonad {
    using value_type = int;
    // ... implement bind and pure
};

DO(x, MyMonad{...}, ...)  // Works!
```

### Q: Can I mix different monads?

**A:** No. All bindings must be the same monad type. Use monad transformers for mixing.

### Q: Is this standard C++?

**A:** Yes, uses only standard C++20 + preprocessor macros. The MBEGIN/MEND syntax requires GCC/Clang extensions.

### Q: What about performance?

**A:** Zero overhead. Compiles to same code as manual bind.

### Q: Can I use this in production?

**A:** Absolutely! It's just syntactic sugar over existing bind operations.

### Q: How do I report bugs?

**A:** Open an issue with:
- Compiler version
- Minimal reproducible example
- Expected vs actual behavior

---

## Further Reading

- [Haskell Do-Notation](https://en.wikibooks.org/wiki/Haskell/do_notation)
- [FP++20 Monad Guide](./MONAD_GUIDE.md)
- [Monads for Functional Programming - Philip Wadler](https://homepages.inf.ed.ac.uk/wadler/papers/marktoberdorf/baastad.pdf)

---

## License

Part of FP++20 - Functional Programming in C++20
