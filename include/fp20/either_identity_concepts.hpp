// ============================================
// FP++20: Concept Opt-ins for Either and Identity
// ============================================
// This file must be included AFTER the concept definitions
// to properly specialize the opt-in traits.

#pragma once

#include <fp20/concepts/functor.hpp>
#include <fp20/concepts/applicative.hpp>
#include <fp20/concepts/monad.hpp>
#include <fp20/either.hpp>
#include <fp20/identity.hpp>

namespace fp20::concepts {
    // ============================================
    // EITHER CONCEPT OPT-IN
    // ============================================
    // Opt-in Either as a Functor
    template<typename L, typename R>
    struct is_functor_opt_in<fp20::Either<L, R>> : std::true_type {};

    // Opt-in Either as an Applicative
    template<typename L, typename R>
    struct is_applicative_opt_in<fp20::Either<L, R>> : std::true_type {};

    // Opt-in Either as a Monad
    template<typename L, typename R>
    struct is_monad_opt_in<fp20::Either<L, R>> : std::true_type {};

    // ============================================
    // IDENTITY CONCEPT OPT-IN
    // ============================================
    // Opt-in Identity as a Functor
    template<typename T>
    struct is_functor_opt_in<fp20::Identity<T>> : std::true_type {};

    // Opt-in Identity as an Applicative
    template<typename T>
    struct is_applicative_opt_in<fp20::Identity<T>> : std::true_type {};

    // Opt-in Identity as a Monad
    template<typename T>
    struct is_monad_opt_in<fp20::Identity<T>> : std::true_type {};
}