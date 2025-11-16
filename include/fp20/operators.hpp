#pragma once

#include <functional>
#include <type_traits>

namespace fp20 {
namespace operators {

// ============================================================================
// FUNCTOR OPERATORS - <$>, <&>
// ============================================================================

// <$> : fmap operator (Haskell's <$>)
// f <$> m  ==  fmap(f, m)
template<typename Func>
struct FmapOperator {
    Func f;

    template<typename M>
    auto operator|(const M& m) const {
        return fmap(f, m);
    }
};

template<typename Func>
auto operator<(Func f, auto (*)(int)) {
    return FmapOperator<Func>{f};
}

// Helper for creating <$>
inline auto fmap_op = [](auto f) {
    return [f](auto m) { return fmap(f, m); };
};

// ============================================================================
// APPLICATIVE OPERATORS - <*>, *>, <*, <**>
// ============================================================================

// <*> : apply operator (Haskell's <*>)
// mf <*> mx  ==  apply(mf, mx)
template<typename MF>
struct ApplyOperator {
    MF mf;

    template<typename MX>
    auto operator*(const MX& mx) const {
        return apply(mf, mx);
    }
};

// Lifted operators
// liftA2 f x y = f <$> x <*> y
template<typename F, typename MA, typename MB>
auto liftA2(F f, const MA& ma, const MB& mb) {
    return apply(fmap(f, ma), mb);
}

template<typename F, typename MA, typename MB, typename MC>
auto liftA3(F f, const MA& ma, const MB& mb, const MC& mc) {
    return apply(apply(fmap(f, ma), mb), mc);
}

// ============================================================================
// MONAD OPERATORS - >>=, >>, <=<, >=>
// ============================================================================

// >>= : bind operator (Haskell's >>=)
template<typename M>
struct BindOperator {
    M m;

    template<typename F>
    auto operator>>=(F f) const {
        return bind(m, f);
    }
};

// >> : then operator (Haskell's >>)
// m1 >> m2  ==  m1 >>= (\_ -> m2)
template<typename M1>
struct ThenOperator {
    M1 m1;

    template<typename M2>
    auto operator>>(const M2& m2) const {
        return bind(m1, [m2](auto) { return m2; });
    }
};

// <=< : Kleisli composition (right-to-left)
// (g <=< f) x  ==  f(x) >>= g
template<typename G>
struct KleisliLeft {
    G g;

    template<typename F>
    auto operator<(F f) const {
        return [g = this->g, f](auto x) {
            return bind(f(x), g);
        };
    }
};

// >=> : Kleisli composition (left-to-right)
// (f >=> g) x  ==  f(x) >>= g
template<typename F>
struct KleisliRight {
    F f;

    template<typename G>
    auto operator>(G g) const {
        return [f = this->f, g](auto x) {
            return bind(f(x), g);
        };
    }
};

// ============================================================================
// MONADPLUS OPERATORS - <|>, mzero, mplus
// ============================================================================

// <|> : choice operator (Haskell's <|>)
template<typename M>
struct ChoiceOperator {
    M m1;

    template<typename M2>
    auto operator|(const M2& m2) const {
        return mplus(m1, m2);
    }
};

// ============================================================================
// CONTINUATION OPERATORS - callCC helpers
// ============================================================================

// Escape combinator for callCC
template<typename R, typename A>
auto escape_with(A value) {
    return [value](auto escape) {
        return escape(value);
    };
}

// ============================================================================
// COMPOSITION OPERATORS - ., $
// ============================================================================

// Function composition: (g . f)(x) = g(f(x))
template<typename G>
struct Compose {
    G g;

    template<typename F>
    auto operator*(F f) const {
        return [g = this->g, f](auto x) {
            return g(f(x));
        };
    }
};

template<typename G>
auto compose(G g) {
    return Compose<G>{g};
}

// $ operator: f $ x = f(x) (low precedence application)
template<typename F>
struct Apply {
    F f;

    template<typename X>
    auto operator%(X x) const {
        return f(x);
    }
};

// ============================================================================
// PIPELINE OPERATORS - |>, <|
// ============================================================================

// |> : forward pipe (x |> f = f(x))
template<typename X>
struct ForwardPipe {
    X x;

    template<typename F>
    auto operator|(F f) const {
        return f(x);
    }
};

// <| : backward pipe (f <| x = f(x))
template<typename F>
struct BackwardPipe {
    F f;

    template<typename X>
    auto operator|(X x) const {
        return f(x);
    }
};

// ============================================================================
// DO-NOTATION SIMULATION (Monadic let)
// ============================================================================

// Monadic let binding helper
#define MLET(var, expr) \
    [&](auto var) -> decltype(auto)

#define MRETURN(expr) \
    return pure(expr); \
    }

// Example usage:
// auto computation =
//     bind(getX(), MLET(x,
//     bind(getY(), MLET(y,
//     MRETURN(x + y)
//     ))));

// ============================================================================
// SMART CONSTRUCTORS
// ============================================================================

// Wrap value in monad (smart pure)
template<template<typename...> class M, typename A>
auto just(A value) {
    return pure<M>(value);
}

// Nothing/Empty for Maybe-like monads
template<template<typename...> class M, typename A>
auto nothing() {
    return mzero<M, A>();
}

// Left/Right for Either
template<typename E>
auto left(E error) {
    return [error](auto) {
        return Either<E, decltype(error)>::Left(error);
    };
}

template<typename A>
auto right(A value) {
    return [value](auto) {
        return Either<decltype(value), A>::Right(value);
    };
}

} // namespace operators

// ============================================================================
// OPERATOR INJECTION FOR MONADS
// ============================================================================

// Enable operators for all monads via ADL
template<typename M>
    requires concepts::Monad<M>
auto operator>>=(const M& m, auto f) {
    return bind(m, f);
}

template<typename M1, typename M2>
    requires concepts::Monad<M1> && concepts::Monad<M2>
auto operator>>(const M1& m1, const M2& m2) {
    return bind(m1, [m2](auto) { return m2; });
}

// Functor <$>
template<typename F, typename M>
    requires concepts::Functor<M>
auto operator|(F f, const M& m) {
    return fmap(f, m);
}

// MonadPlus <|>
template<typename M1, typename M2>
    requires concepts::MonadPlus<M1>
auto operator||(const M1& m1, const M2& m2) {
    return mplus(m1, m2);
}

} // namespace fp20
