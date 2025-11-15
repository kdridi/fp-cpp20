# State Monad Implementation Guide (GREEN Phase)

## Quick Reference for Implementation

### File Structure

```
include/fp20/
├── state.hpp              # Main State<S,A> implementation (CREATE THIS)
└── state_concepts.hpp     # Concept opt-ins (CREATE THIS)
```

### Implementation Order

1. State<S,A> class template
2. State primitives (get, put, modify, gets)
3. Functor instance (fmap)
4. Applicative instance (pure, apply)
5. Monad instance (bind)
6. Helper functions (evalState, execState)
7. Concept opt-ins

---

## 1. State<S,A> Class Template

**File**: `/Users/kdridi/Documents/fp++20/include/fp20/state.hpp`

```cpp
#pragma once

#include <functional>
#include <utility>

namespace fp20 {

template<typename S, typename A>
class State {
public:
    using state_type = S;
    using value_type = A;

private:
    std::function<std::pair<A, S>(S)> runStateF;

public:
    // Constructor: accepts function S -> (A, S)
    explicit State(std::function<std::pair<A, S>(S)> f)
        : runStateF(std::move(f)) {}

    // Also accept any callable (lambda, function object)
    template<typename F>
    requires std::invocable<F, S> &&
             std::same_as<std::invoke_result_t<F, S>, std::pair<A, S>>
    explicit State(F&& f)
        : runStateF(std::forward<F>(f)) {}

    // runState: execute the stateful computation
    std::pair<A, S> runState(S s) const {
        return runStateF(s);
    }

    // Copy/move semantics (defaulted)
    State(const State&) = default;
    State(State&&) = default;
    State& operator=(const State&) = default;
    State& operator=(State&&) = default;
};

} // namespace fp20
```

**Tests this satisfies**: 12 tests in `test_state_construction`

---

## 2. State Primitives

**Add to**: `/Users/kdridi/Documents/fp++20/include/fp20/state.hpp`

```cpp
namespace fp20 {

// get :: State s s
// Returns current state as the result value
template<typename S>
State<S, S> get() {
    return State<S, S>([](S s) -> std::pair<S, S> {
        return {s, s};
    });
}

// put :: s -> State s ()
// Sets new state, returns unit (we'll use a Unit type or std::monostate)
struct Unit {}; // Monoidal unit type

template<typename S>
State<S, Unit> put(S new_state) {
    return State<S, Unit>([new_state](S) -> std::pair<Unit, S> {
        return {Unit{}, new_state};
    });
}

// modify :: (s -> s) -> State s ()
// Applies transformation to state
template<typename S, typename F>
requires std::invocable<F, S> && std::same_as<std::invoke_result_t<F, S>, S>
State<S, Unit> modify(F&& f) {
    return State<S, Unit>([f = std::forward<F>(f)](S s) -> std::pair<Unit, S> {
        return {Unit{}, f(s)};
    });
}

// gets :: (s -> a) -> State s a
// Projects a value from the state
template<typename S, typename F>
requires std::invocable<F, S>
auto gets(F&& f) -> State<S, std::invoke_result_t<F, S>> {
    using A = std::invoke_result_t<F, S>;
    return State<S, A>([f = std::forward<F>(f)](S s) -> std::pair<A, S> {
        return {f(s), s};
    });
}

} // namespace fp20
```

**Tests this satisfies**: 15 tests in `test_state_primitives`

---

## 3. Functor Instance

**Add to**: `/Users/kdridi/Documents/fp++20/include/fp20/state.hpp`

```cpp
namespace fp20 {

// fmap for State: transforms the result value, preserves state threading
template<typename F, typename S, typename A>
requires std::invocable<F, A>
auto fmap(F&& f, const State<S, A>& state) {
    using B = std::invoke_result_t<F, A>;

    return State<S, B>([state, f = std::forward<F>(f)](S s) -> std::pair<B, S> {
        auto [a, s_new] = state.runState(s);
        return {f(a), s_new};
    });
}

} // namespace fp20
```

**Tests this satisfies**: 8 tests in `test_state_functor`

---

## 4. Applicative Instance

**Add to**: `/Users/kdridi/Documents/fp++20/include/fp20/state.hpp`

```cpp
namespace fp20 {

// pure / return_ for State: creates stateless computation
template<template<typename...> class M, typename S, typename A>
requires std::same_as<M<S, A>, State<S, A>>
State<S, A> pure(A a) {
    return State<S, A>([a](S s) -> std::pair<A, S> {
        return {a, s};
    });
}

template<template<typename...> class M, typename S, typename A>
requires std::same_as<M<S, A>, State<S, A>>
State<S, A> return_(A a) {
    return pure<M, S, A>(a);
}

// apply for State: threads state through function application
template<typename S, typename A, typename B>
auto apply(const State<S, std::function<B(A)>>& sf, const State<S, A>& sa) {
    return State<S, B>([sf, sa](S s) -> std::pair<B, S> {
        auto [f, s1] = sf.runState(s);
        auto [a, s2] = sa.runState(s1);
        return {f(a), s2};
    });
}

// More general apply for any callable
template<typename S, typename F, typename A>
requires std::invocable<F, A>
auto apply(const State<S, F>& sf, const State<S, A>& sa) {
    using B = std::invoke_result_t<F, A>;

    return State<S, B>([sf, sa](S s) -> std::pair<B, S> {
        auto [f, s1] = sf.runState(s);
        auto [a, s2] = sa.runState(s1);
        return {f(a), s2};
    });
}

} // namespace fp20
```

**Tests this satisfies**: 6 tests in `test_state_applicative`

---

## 5. Monad Instance

**Add to**: `/Users/kdridi/Documents/fp++20/include/fp20/state.hpp`

```cpp
namespace fp20 {

// bind for State: threads state sequentially through computations
template<typename S, typename A, typename F>
requires std::invocable<F, A>
auto bind(const State<S, A>& m, F&& k) {
    using StateB = std::invoke_result_t<F, A>;
    using B = typename StateB::value_type;

    return State<S, B>([m, k = std::forward<F>(k)](S s) -> std::pair<B, S> {
        auto [a, s_new] = m.runState(s);
        return k(a).runState(s_new);
    });
}

// Operator >>= for convenience (optional)
template<typename S, typename A, typename F>
auto operator>>=(const State<S, A>& m, F&& k) {
    return bind(m, std::forward<F>(k));
}

} // namespace fp20
```

**Tests this satisfies**: 9 tests in `test_state_monad`, 10 tests in `test_state_laws`

---

## 6. Helper Functions

**Add to**: `/Users/kdridi/Documents/fp++20/include/fp20/state.hpp`

```cpp
namespace fp20 {

// evalState :: State s a -> s -> a
// Runs computation and returns only the result value
template<typename S, typename A>
A evalState(const State<S, A>& state, S initial_state) {
    auto [value, _] = state.runState(initial_state);
    return value;
}

// execState :: State s a -> s -> s
// Runs computation and returns only the final state
template<typename S, typename A>
S execState(const State<S, A>& state, S initial_state) {
    auto [_, final_state] = state.runState(initial_state);
    return final_state;
}

} // namespace fp20
```

**Tests this satisfies**: 6 tests in `test_state_advanced`, 4 runtime tests

---

## 7. Concept Opt-ins

**File**: `/Users/kdridi/Documents/fp++20/include/fp20/state_concepts.hpp`

```cpp
#pragma once

#include <fp20/state.hpp>
#include <fp20/concepts/functor.hpp>
#include <fp20/concepts/applicative.hpp>
#include <fp20/concepts/monad.hpp>

namespace fp20::concepts {

// Opt State into Functor concept
template<typename S, typename A>
struct is_functor<State<S, A>> : std::true_type {};

// Opt State into Applicative concept
template<typename S, typename A>
struct is_applicative<State<S, A>> : std::true_type {};

// Opt State into Monad concept
template<typename S, typename A>
struct is_monad<State<S, A>> : std::true_type {};

} // namespace fp20::concepts
```

**Tests this satisfies**: Concept satisfaction tests in all test namespaces

---

## Implementation Checklist

- [ ] Create `/Users/kdridi/Documents/fp++20/include/fp20/state.hpp`
  - [ ] State<S,A> class template with runState
  - [ ] Type aliases (state_type, value_type)
  - [ ] Constructor accepting callable
  - [ ] Copy/move semantics

- [ ] Implement primitives
  - [ ] get<S>() returns State<S, S>
  - [ ] put<S>(s) returns State<S, Unit>
  - [ ] modify<S>(f) returns State<S, Unit>
  - [ ] gets<S>(f) returns State<S, B>

- [ ] Implement Functor instance
  - [ ] fmap(f, State<S,A>) -> State<S,B>

- [ ] Implement Applicative instance
  - [ ] pure<State, S>(a) -> State<S, A>
  - [ ] return_<State, S>(a) -> State<S, A>
  - [ ] apply(State<S,F>, State<S,A>) -> State<S,B>

- [ ] Implement Monad instance
  - [ ] bind(State<S,A>, f) -> State<S,B>
  - [ ] Optional: operator>>= for bind

- [ ] Implement helper functions
  - [ ] evalState(state, s) -> A
  - [ ] execState(state, s) -> S

- [ ] Create `/Users/kdridi/Documents/fp++20/include/fp20/state_concepts.hpp`
  - [ ] is_functor<State<S,A>> opt-in
  - [ ] is_applicative<State<S,A>> opt-in
  - [ ] is_monad<State<S,A>> opt-in

- [ ] Verify all tests pass
  - [ ] Compile tests: `g++ -std=c++20 -I./include tests/compilation/test_state_monad.cpp`
  - [ ] 62 static_assert should all pass
  - [ ] 28 runtime assertions should all pass
  - [ ] Monad laws should hold

---

## Key Implementation Notes

### 1. State Threading
State must be threaded sequentially:
```cpp
// In bind:
auto [a, s_new] = m.runState(s);      // First computation gets initial state
return k(a).runState(s_new);          // Second computation gets updated state
```

### 2. Type Deduction
Use C++20 concepts and requires clauses for type safety:
```cpp
template<typename S, typename F>
requires std::invocable<F, S> && std::same_as<std::invoke_result_t<F, S>, S>
State<S, Unit> modify(F&& f);
```

### 3. Lambda Captures
Be careful with captures in State constructors:
```cpp
// Capture by value for state computations
State<S, B>([state, f](S s) { /* ... */ });

// Use std::forward for perfect forwarding
State<S, B>([f = std::forward<F>(f)](S s) { /* ... */ });
```

### 4. Unit Type
For operations that don't return a meaningful value (put, modify):
```cpp
struct Unit {};  // Or use std::monostate
```

### 5. Monad Laws
Ensure these hold:
- Left identity: `runState(return(a) >>= f, s) == runState(f(a), s)`
- Right identity: `runState(m >>= return, s) == runState(m, s)`
- Associativity: `runState((m >>= f) >>= g, s) == runState(m >>= (\x -> f(x) >>= g), s)`

---

## Testing Strategy

### After Each Step
```bash
# Compile and run tests
g++ -std=c++20 -I./include tests/compilation/test_state_monad.cpp -o /tmp/test_state && /tmp/test_state
```

### Watch for Errors
1. First: Include errors -> Create files
2. Second: Type errors -> Implement types
3. Third: Function errors -> Implement functions
4. Fourth: Concept errors -> Add opt-ins
5. Finally: Assertion errors -> Fix logic

### Success Criteria
All tests pass when you see:
```
[No compilation errors]
[No assertion failures]
$ echo $?
0
```

---

## Example Usage (from tests)

### Counter Example
```cpp
auto increment = []() {
    return fp20::bind(get<int>(), [](int s) {
        return fp20::bind(put<int>(s + 1), [s](auto) {
            return return_<State, int>(s);
        });
    });
};

auto computation = increment();
auto [value, new_state] = computation.runState(0);
// value == 0 (old state)
// new_state == 1 (incremented)
```

### Accumulator Example
```cpp
auto addToState = [](int x) {
    return modify<int>([x](int s) { return s + x; });
};

auto computation = fp20::bind(addToState(10), [addToState](auto) {
    return fp20::bind(addToState(20), [addToState](auto) {
        return fp20::bind(addToState(30), [](auto) {
            return get<int>();
        });
    });
});

auto final_value = evalState(computation, 0);
// final_value == 60
```

---

## File Paths Summary

**Test file**: `/Users/kdridi/Documents/fp++20/tests/compilation/test_state_monad.cpp`

**Files to create**:
- `/Users/kdridi/Documents/fp++20/include/fp20/state.hpp`
- `/Users/kdridi/Documents/fp++20/include/fp20/state_concepts.hpp`

**Test dependencies** (should exist):
- `/Users/kdridi/Documents/fp++20/include/fp20/concepts/functor.hpp`
- `/Users/kdridi/Documents/fp++20/include/fp20/concepts/applicative.hpp`
- `/Users/kdridi/Documents/fp++20/include/fp20/concepts/monad.hpp`
