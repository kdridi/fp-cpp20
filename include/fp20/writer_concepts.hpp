// ============================================================================
// FP++20: Concept Opt-ins for Writer<W, A> Monad
// ============================================================================
// This file must be included AFTER the concept definitions to properly
// specialize the opt-in traits for Writer<W, A>.
//
// The Writer monad satisfies Functor, Applicative, and Monad concepts through
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
// This prevents false positives while allowing extensibility. Writer<W, A>
// must explicitly declare it satisfies these concepts, even though it has
// the required structural interface (value_type, log_type, etc.).
//
// ============================================================================
// WRITER MONAD HIERARCHY
// ============================================================================
//
// Writer<W, A> implements the full monadic hierarchy:
//
// 1. Functor:
//    - fmap transforms the value while preserving the log
//    - fmap :: (a → b) → Writer w a → Writer w b
//    - Pattern: fmap(f, Writer(a, w)) = Writer(f(a), w)
//
// 2. Applicative:
//    - pure creates Writer with empty log (mempty)
//    - apply combines values and concatenates logs with mappend
//    - pure :: a → Writer w a
//    - apply :: Writer w (a → b) → Writer w a → Writer w b
//    - Pattern: Writer(f, w1) <*> Writer(a, w2) = Writer(f(a), w1 <> w2)
//
// 3. Monad:
//    - bind chains computations and accumulates logs
//    - bind :: Writer w a → (a → Writer w b) → Writer w b
//    - Pattern: Writer(a, w1) >>= f = let Writer(b, w2) = f(a) in Writer(b, w1 <> w2)
//
// KEY CONSTRAINT: W must be a Monoid for log concatenation.
//
// ============================================================================

#pragma once

#include <fp20/concepts/functor.hpp>
#include <fp20/concepts/applicative.hpp>
#include <fp20/concepts/monad.hpp>
#include <fp20/writer.hpp>

namespace fp20::concepts {

    // ========================================================================
    // WRITER FUNCTOR OPT-IN
    // ========================================================================
    // Writer<W, A> is a Functor: we can transform the value type A while
    // preserving the log type W and its accumulated content.
    //
    // fmap :: (a → b) → Writer w a → Writer w b
    // fmap f (Writer a w) = Writer (f a) w
    //
    // The log remains unchanged during functor mapping.
    template<typename W, typename A>
    struct is_functor_opt_in<fp20::Writer<W, A>> : std::true_type {};

    // ========================================================================
    // WRITER APPLICATIVE OPT-IN
    // ========================================================================
    // Writer<W, A> is an Applicative: we can lift pure values (with empty log)
    // and apply functions in Writer context while concatenating logs.
    //
    // pure :: a → Writer w a
    // pure a = Writer a mempty
    //
    // apply :: Writer w (a → b) → Writer w a → Writer w b
    // apply (Writer f w1) (Writer a w2) = Writer (f a) (w1 <> w2)
    //
    // Log concatenation uses the Monoid operation mappend.
    template<typename W, typename A>
    struct is_applicative_opt_in<fp20::Writer<W, A>> : std::true_type {};

    // ========================================================================
    // WRITER MONAD OPT-IN
    // ========================================================================
    // Writer<W, A> is a Monad: we can sequence computations that produce
    // values and logs, with automatic log accumulation via monoidal append.
    //
    // return :: a → Writer w a
    // return a = Writer a mempty
    //
    // bind :: Writer w a → (a → Writer w b) → Writer w b
    // bind (Writer a w1) k = let Writer b w2 = k a in Writer b (w1 <> w2)
    //
    // The key insight: logs from all computations in a bind chain are
    // concatenated left-to-right using mappend, enabling compositional logging.
    template<typename W, typename A>
    struct is_monad_opt_in<fp20::Writer<W, A>> : std::true_type {};

} // namespace fp20::concepts

// Export concepts to fp20 namespace for convenience
namespace fp20 {
    using fp20::concepts::Functor;
    using fp20::concepts::Applicative;
    using fp20::concepts::Monad;
} // namespace fp20
