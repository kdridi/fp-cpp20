// ============================================
// FP++20: Concept Opt-ins for State<S,A>
// ============================================
// This file must be included AFTER the concept definitions
// to properly specialize the opt-in traits.
//
// State<S,A> implements the full monad hierarchy:
// - Functor: fmap transforms result values
// - Applicative: pure/apply thread state through applications
// - Monad: bind sequences stateful computations

#pragma once

#include <fp20/state.hpp>
#include <fp20/concepts/functor.hpp>
#include <fp20/concepts/applicative.hpp>
#include <fp20/concepts/monad.hpp>

namespace fp20::concepts {
    // ============================================
    // STATE FUNCTOR OPT-IN
    // ============================================
    // State<S,A> is a Functor: fmap transforms the result value
    // while preserving state threading semantics.
    template<typename S, typename A>
    struct is_functor_opt_in<fp20::State<S, A>> : std::true_type {};

    // ============================================
    // STATE APPLICATIVE OPT-IN
    // ============================================
    // State<S,A> is an Applicative: pure creates stateless computations,
    // apply threads state through function application.
    template<typename S, typename A>
    struct is_applicative_opt_in<fp20::State<S, A>> : std::true_type {};

    // ============================================
    // STATE MONAD OPT-IN
    // ============================================
    // State<S,A> is a Monad: bind threads state sequentially through
    // data-dependent computations, enabling stateful workflows.
    template<typename S, typename A>
    struct is_monad_opt_in<fp20::State<S, A>> : std::true_type {};
}
