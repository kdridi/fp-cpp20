// ============================================================================
// FP++20: Functional Programming Library for Modern C++
// ============================================================================
// A comprehensive C++20 library bringing Haskell-style functional programming
// to modern C++. Featuring monads, applicatives, functors, and more.
//
// Single include file - Include this to get all FP++20 functionality
//
// Author: Generated with Claude Code
// License: MIT
// ============================================================================

#pragma once

// ============================================================================
// CONCEPTS - Type Classes
// ============================================================================
#include <fp20/concepts/functor.hpp>
#include <fp20/concepts/applicative.hpp>
#include <fp20/concepts/monad.hpp>
#include <fp20/concepts/monoid.hpp>

// ============================================================================
// MONADS - Computational Structures
// ============================================================================
#include <fp20/monads/identity.hpp>
#include <fp20/monads/either.hpp>
#include <fp20/monads/state.hpp>
#include <fp20/monads/reader.hpp>
#include <fp20/monads/writer.hpp>
#include <fp20/monads/io.hpp>
#include <fp20/monads/list.hpp>

// ============================================================================
// CONCEPT OPT-INS - Enable Type Class Instances
// ============================================================================
#include <fp20/concepts/either_identity_concepts.hpp>
#include <fp20/concepts/state_concepts.hpp>
#include <fp20/concepts/reader_concepts.hpp>
#include <fp20/concepts/writer_concepts.hpp>
#include <fp20/concepts/io_concepts.hpp>
#include <fp20/concepts/list_concepts.hpp>

// ============================================================================
// LIBRARY OVERVIEW
// ============================================================================
//
// FP++20 provides:
//
// 1. Type Classes (Concepts):
//    - Functor:     Types that can be mapped over
//    - Applicative: Functors with application and pure
//    - Monad:       Applicatives with bind (>>=)
//    - Monoid:      Types with identity and associative binary operation
//
// 2. Monad Implementations:
//    - Identity<A>:    Simplest monad (wrapper)
//    - Either<L,R>:    Error handling without exceptions
//    - State<S,A>:     Stateful computations
//    - Reader<E,A>:    Environment/dependency injection
//    - Writer<W,A>:    Logging and accumulation
//    - IO<A>:          Side effects with referential transparency
//    - List<A>:        Non-deterministic computation (std::vector)
//
// 3. Operations:
//    - fmap:   Transform values inside functors
//    - pure:   Lift values into applicative/monad
//    - apply:  Apply wrapped functions to wrapped values
//    - bind:   Sequence monadic computations (>>=)
//
// 4. Utilities:
//    - Monad laws verification
//    - Academic citations and references
//    - Comprehensive test suites
//
// ============================================================================
// USAGE EXAMPLES
// ============================================================================
//
// Example 1: Maybe-style error handling with Either
//   auto divide = [](int a, int b) -> Either<std::string, int> {
//     if (b == 0)
//       return left<int>(std::string{"Division by zero"});
//     return right<std::string>(a / b);
//   };
//
// Example 2: Stateful computation
//   auto computation = bind(get<int>(), [](int s) {
//     return bind(put(s + 1), [s]() {
//       return pure<State, int>(s * 2);
//     });
//   });
//
// Example 3: IO with side effects
//   auto program = bind(putStrLn("Enter name: "), []() {
//     return bind(getLine(), [](std::string name) {
//       return putStrLn("Hello, " + name);
//     });
//   });
//   program.unsafeRun();  // Execute effects
//
// Example 4: List comprehensions (Pythagorean triples)
//   auto triples = bind(range(1,20), [](int a) {
//     return bind(range(a,20), [=](int b) {
//     return bind(range(b,20), [=](int c) {
//     return bind(guard<int>(a*a + b*b == c*c), [=](int) {
//       return pure<List>(std::tuple{a,b,c});
//     });});});
//   });
//
// ============================================================================

namespace fp20 {
    // All types and functions are exported from their respective headers
    // Simply include <fp20/fp20.hpp> to access everything
}
