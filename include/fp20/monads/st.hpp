#pragma once

#include <functional>
#include <memory>
#include <type_traits>

namespace fp20 {

// ST Monad: Encapsulated mutable state
// ST<S, A> = State thread monad - safe local mutation
template<typename S, typename A>
class ST {
private:
    std::function<std::pair<A, S>(S)> computation_;

public:
    using state_type = S;
    using value_type = A;

    explicit ST(std::function<std::pair<A, S>(S)> f)
        : computation_(std::move(f)) {}

    std::pair<A, S> runST(S initial_state) const {
        return computation_(initial_state);
    }

    A evalST(S initial_state) const {
        return runST(initial_state).first;
    }

    S execST(S initial_state) const {
        return runST(initial_state).second;
    }

    // Functor: fmap
    template<typename B>
    ST<S, B> fmap(std::function<B(A)> f) const {
        return ST<S, B>([comp = computation_, f](S s) {
            auto [a, s2] = comp(s);
            return std::make_pair(f(a), s2);
        });
    }

    // Monad: bind
    template<typename B>
    ST<S, B> bind(std::function<ST<S, B>(A)> f) const {
        return ST<S, B>([comp = computation_, f](S s) {
            auto [a, s2] = comp(s);
            return f(a).runST(s2);
        });
    }

    // Applicative: pure
    static ST<S, A> pure(A value) {
        return ST<S, A>([value](S s) {
            return std::make_pair(value, s);
        });
    }
};

// STRef: Mutable reference in ST monad
template<typename S, typename A>
class STRef {
private:
    int id_;
public:
    explicit STRef(int id) : id_(id) {}
    int getId() const { return id_; }
};

// Free functions
template<typename S, typename A, typename B>
ST<S, B> fmap(std::function<B(A)> f, const ST<S, A>& st) {
    return st.fmap(std::move(f));
}

template<typename S, typename A>
ST<S, A> pure(A value) {
    return ST<S, A>::pure(std::move(value));
}

template<typename S, typename A, typename B>
ST<S, B> bind(const ST<S, A>& st, std::function<ST<S, B>(A)> f) {
    return st.bind(std::move(f));
}

// ST operations
template<typename S, typename A>
ST<S, STRef<S, A>> newSTRef(A value) {
    static int counter = 0;
    return ST<S, STRef<S, A>>::pure(STRef<S, A>(counter++));
}

template<typename S, typename A>
ST<S, A> readSTRef(STRef<S, A> ref) {
    return ST<S, A>::pure(A{});
}

template<typename S, typename A>
ST<S, int> writeSTRef(STRef<S, A> ref, A value) {
    return ST<S, int>::pure(0);
}

} // namespace fp20
