// ============================================================================
// FP++20: Concept Opt-ins for IO<A> Monad
// ============================================================================
// This file must be included AFTER the concept definitions to properly
// specialize the opt-in traits for IO<A>.
//
// The IO monad satisfies Functor, Applicative, and Monad concepts through
// explicit opt-in, as it's a custom type not recognized by pattern matching.
//
// ============================================================================
// DESIGN RATIONALE
// ============================================================================
//
// The fp20 concept system uses a hybrid approach:
// 1. Pattern matching for standard library types (std::vector, std::optional)
// 2. Explicit opt-in for custom types via is_*_opt_in traits
//
// This prevents false positives while allowing extensibility. IO<A>
// must explicitly declare it satisfies these concepts, even though it has
// the required structural interface (value_type, etc.).
//
// IO MONAD CONCEPT HIERARCHY:
// - Functor: fmap transforms eventual results without introducing effects
// - Applicative: pure/apply sequence effects while applying functions
// - Monad: bind enables sequential composition with data-dependent effects
//
// ============================================================================
// IO MONAD CHARACTERISTICS
// ============================================================================
//
// The IO monad is special among monads because:
// 1. It represents the "base" of the effect stack in most programs
// 2. It cannot be "unwrapped" except at program boundaries (main)
// 3. It enforces lazy evaluation - effects ONLY happen at unsafeRun()
// 4. It provides type-level effect tracking (pure vs. effectful code)
//
// Academic Reference:
// - [Lazy Functional State Threads] - Launchbury & Peyton Jones (1994)
//   https://www.microsoft.com/en-us/research/publication/lazy-functional-state-threads/
//   Foundation for understanding IO as a state transformer
//
// ============================================================================

#pragma once

#include <fp20/concepts/functor.hpp>
#include <fp20/concepts/applicative.hpp>
#include <fp20/concepts/monad.hpp>
#include <fp20/monads/io.hpp>

namespace fp20::concepts {

    // ========================================================================
    // IO FUNCTOR OPT-IN
    // ========================================================================
    // IO<A> is a Functor: we can transform the eventual result type A
    // without introducing new effects or executing existing ones.
    //
    // fmap :: (A → B) → IO A → IO B
    //
    // LAZY SEMANTICS: fmap constructs a NEW IO action that, when run,
    // will execute the original action and transform the result.
    // No effects occur during fmap itself.
    //
    // Example:
    //   IO<int> readNum = effect(readInt);
    //   IO<string> readStr = fmap(to_string, readNum);
    //   // Nothing executed yet - both are suspended computations
    template<typename A>
    struct is_functor_opt_in<fp20::IO<A>> : std::true_type {};

    // ========================================================================
    // IO APPLICATIVE OPT-IN
    // ========================================================================
    // IO<A> is an Applicative: we can lift pure values and apply
    // functions in IO context while sequencing effects properly.
    //
    // pure :: A → IO A
    // apply :: IO (A → B) → IO A → IO B
    //
    // SEQUENCING SEMANTICS: apply executes the function IO first,
    // then the argument IO, then applies the function to the value.
    // This enforces left-to-right effect ordering.
    //
    // Example:
    //   IO<function<int(int)>> f = pure([](int x) { return x * 2; });
    //   IO<int> a = effect(readInt);
    //   IO<int> result = apply(f, a);
    //   // Still suspended - no effects yet
    template<typename A>
    struct is_applicative_opt_in<fp20::IO<A>> : std::true_type {};

    // ========================================================================
    // IO MONAD OPT-IN
    // ========================================================================
    // IO<A> is a Monad: we can sequence effectful computations where
    // later actions depend on the results of earlier ones.
    //
    // return :: A → IO A
    // bind :: IO A → (A → IO B) → IO B
    //
    // MONADIC COMPOSITION: bind is the key operation that distinguishes
    // IO from simpler abstractions. It enables:
    // - Data-dependent effects (next action depends on previous result)
    // - Sequential effect composition (maintain execution order)
    // - Nested IO flattening (IO<IO<A>> becomes IO<A>)
    //
    // CRITICAL PROPERTY: All composition is LAZY.
    // bind creates a new IO description; no effects execute until unsafeRun().
    //
    // Example:
    //   auto program = bind(
    //       readLine(),
    //       [](string line) {
    //           return writeLine("You said: " + line);
    //       }
    //   );
    //   // Still just a description - no I/O performed
    //   program.unsafeRun();  // NOW effects happen
    //
    // Academic Reference:
    // - [Tackling the Awkward Squad] - Simon Peyton Jones (2001)
    //   Section 2: The IO Monad
    //   Definitive guide to IO monad design and semantics
    template<typename A>
    struct is_monad_opt_in<fp20::IO<A>> : std::true_type {};

    // ========================================================================
    // IO<void> CONCEPT OPT-INS
    // ========================================================================
    // Template specialization for IO<void> also satisfies all concepts

    template<>
    struct is_functor_opt_in<fp20::IO<void>> : std::true_type {};

    template<>
    struct is_applicative_opt_in<fp20::IO<void>> : std::true_type {};

    template<>
    struct is_monad_opt_in<fp20::IO<void>> : std::true_type {};

} // namespace fp20::concepts

// Export concepts to fp20 namespace for convenience
namespace fp20 {
    using fp20::concepts::Functor;
    using fp20::concepts::Applicative;
    using fp20::concepts::Monad;
} // namespace fp20
