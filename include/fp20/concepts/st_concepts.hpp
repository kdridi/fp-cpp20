#pragma once
#include <fp20/monads/st.hpp>
#include <fp20/concepts/functor.hpp>
#include <fp20/concepts/applicative.hpp>
#include <fp20/concepts/monad.hpp>

namespace fp20 {
template<typename S, typename A>
struct is_functor_opt_in<ST<S, A>> : std::true_type {};

template<typename S, typename A>
struct is_applicative_opt_in<ST<S, A>> : std::true_type {};

template<typename S, typename A>
struct is_monad_opt_in<ST<S, A>> : std::true_type {};
}
