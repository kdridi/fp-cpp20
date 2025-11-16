#pragma once

#include <optional>
#include <functional>

namespace fp20 {

// ============================================================================
// MAYBET - Maybe monad transformer
// ============================================================================

template<template<typename> class M, typename A>
class MaybeT {
    M<std::optional<A>> value_;
public:
    explicit MaybeT(M<std::optional<A>> v) : value_(v) {}

    M<std::optional<A>> runMaybeT() const { return value_; }

    static MaybeT<M, A> lift(const M<A>& ma) {
        return MaybeT(fmap([](A a) { return std::optional<A>(a); }, ma));
    }

    template<typename B>
    MaybeT<M, B> bind(std::function<MaybeT<M, B>(A)> f) const {
        return MaybeT<M, B>(
            fp20::bind(value_, [f](std::optional<A> opt) {
                if (!opt) return M<std::optional<B>>::pure(std::nullopt);
                return f(*opt).runMaybeT();
            })
        );
    }
};

// ============================================================================
// EITHERT - Either monad transformer
// ============================================================================

template<typename E, template<typename> class M, typename A>
class EitherT {
    M<Either<E, A>> value_;
public:
    explicit EitherT(M<Either<E, A>> v) : value_(v) {}

    M<Either<E, A>> runEitherT() const { return value_; }

    static EitherT<E, M, A> lift(const M<A>& ma) {
        return EitherT(fmap([](A a) { return Either<E, A>::Right(a); }, ma));
    }

    template<typename B>
    EitherT<E, M, B> bind(std::function<EitherT<E, M, B>(A)> f) const {
        return EitherT<E, M, B>(
            fp20::bind(value_, [f](Either<E, A> e) {
                return e.match(
                    [](E err) { return M<Either<E, B>>::pure(Either<E, B>::Left(err)); },
                    [f](A a) { return f(a).runEitherT(); }
                );
            })
        );
    }
};

// ============================================================================
// STATET - State monad transformer
// ============================================================================

template<typename S, template<typename> class M, typename A>
class StateT {
    std::function<M<std::pair<A, S>>(S)> runStateT_;
public:
    explicit StateT(std::function<M<std::pair<A, S>>(S)> f) : runStateT_(f) {}

    M<std::pair<A, S>> runStateT(S s) const { return runStateT_(s); }

    static StateT<S, M, A> lift(const M<A>& ma) {
        return StateT([ma](S s) {
            return fmap([s](A a) { return std::make_pair(a, s); }, ma);
        });
    }

    template<typename B>
    StateT<S, M, B> bind(std::function<StateT<S, M, B>(A)> f) const {
        return StateT<S, M, B>([this, f](S s) {
            return fp20::bind(this->runStateT_(s), [f](std::pair<A, S> p) {
                return f(p.first).runStateT(p.second);
            });
        });
    }
};

// ============================================================================
// READERT - Reader monad transformer
// ============================================================================

template<typename R, template<typename> class M, typename A>
class ReaderT {
    std::function<M<A>(R)> runReaderT_;
public:
    explicit ReaderT(std::function<M<A>(R)> f) : runReaderT_(f) {}

    M<A> runReaderT(R r) const { return runReaderT_(r); }

    static ReaderT<R, M, A> lift(const M<A>& ma) {
        return ReaderT([ma](R) { return ma; });
    }

    template<typename B>
    ReaderT<R, M, B> bind(std::function<ReaderT<R, M, B>(A)> f) const {
        return ReaderT<R, M, B>([this, f](R r) {
            return fp20::bind(this->runReaderT_(r), [f, r](A a) {
                return f(a).runReaderT(r);
            });
        });
    }
};

// ============================================================================
// WRITERT - Writer monad transformer
// ============================================================================

template<typename W, template<typename> class M, typename A>
class WriterT {
    M<std::pair<A, W>> value_;
public:
    explicit WriterT(M<std::pair<A, W>> v) : value_(v) {}

    M<std::pair<A, W>> runWriterT() const { return value_; }

    static WriterT<W, M, A> lift(const M<A>& ma) {
        return WriterT(fmap([](A a) { return std::make_pair(a, mempty<W>()); }, ma));
    }

    template<typename B>
    WriterT<W, M, B> bind(std::function<WriterT<W, M, B>(A)> f) const {
        return WriterT<W, M, B>(
            fp20::bind(value_, [f](std::pair<A, W> p) {
                return fmap([w1 = p.second](std::pair<B, W> p2) {
                    return std::make_pair(p2.first, mappend(w1, p2.second));
                }, f(p.first).runWriterT());
            })
        );
    }
};

} // namespace fp20
