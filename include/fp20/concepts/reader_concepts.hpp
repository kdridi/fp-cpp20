// ============================================================================
// FP++20: Concept Opt-ins for Reader Monad
// ============================================================================
// This file must be included AFTER the concept definitions to properly
// specialize the opt-in traits for Reader<E, A>.
//
// The Reader monad satisfies Functor, Applicative, and Monad concepts through
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
// This prevents false positives while allowing extensibility. Reader<E, A>
// must explicitly declare it satisfies these concepts, even though it has
// the required structural interface (value_type, etc.).
//
// ============================================================================

#pragma once

#include <fp20/concepts/functor.hpp>
#include <fp20/concepts/applicative.hpp>
#include <fp20/concepts/monad.hpp>
#include <fp20/monads/reader.hpp>

namespace fp20::concepts {

    // ========================================================================
    // READER FUNCTOR OPT-IN
    // ========================================================================
    // Reader<E, A> is a Functor: we can transform the result type A while
    // preserving the environment type E.
    //
    // fmap :: (a → b) → Reader e a → Reader e b
    //
    // This is post-composition: fmap f reader = f ∘ reader
    template<typename E, typename A>
    struct is_functor_opt_in<fp20::Reader<E, A>> : std::true_type {};

    // ========================================================================
    // READER APPLICATIVE OPT-IN
    // ========================================================================
    // Reader<E, A> is an Applicative: we can lift pure values and apply
    // functions in Reader context while sharing the environment.
    //
    // pure :: a → Reader e a
    // apply :: Reader e (a → b) → Reader e a → Reader e b
    //
    // The environment is shared: both the function and argument receive
    // the same environment value.
    template<typename E, typename A>
    struct is_applicative_opt_in<fp20::Reader<E, A>> : std::true_type {};

    // ========================================================================
    // READER MONAD OPT-IN
    // ========================================================================
    // Reader<E, A> is a Monad: we can sequence computations that depend on
    // a shared environment, with later computations depending on earlier results.
    //
    // return :: a → Reader e a
    // bind :: Reader e a → (a → Reader e b) → Reader e b
    //
    // All computations in a bind chain receive the same environment value,
    // enabling implicit dependency injection.
    template<typename E, typename A>
    struct is_monad_opt_in<fp20::Reader<E, A>> : std::true_type {};

} // namespace fp20::concepts

// Export concepts to fp20 namespace for convenience
namespace fp20 {
    using fp20::concepts::Functor;
    using fp20::concepts::Applicative;
    using fp20::concepts::Monad;
} // namespace fp20
