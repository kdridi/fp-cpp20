#pragma once

#include <utility>
#include <optional>

namespace fp20::constexpr_monads {

// ============================================================================
// CONSTEXPR MAYBE - MONADS AU COMPILE-TIME !!!
// ============================================================================

template<typename T>
struct Maybe {
    T value;
    bool has_value;

    constexpr Maybe() : value{}, has_value(false) {}
    constexpr Maybe(T v) : value(v), has_value(true) {}

    constexpr bool isJust() const { return has_value; }
    constexpr bool isNothing() const { return !has_value; }

    constexpr T fromJust() const { return value; }

    // MONAD: bind AU COMPILE-TIME
    template<typename F>
    constexpr auto bind(F f) const {
        using R = decltype(f(value));
        if (!has_value) return R{};
        return f(value);
    }

    // MONAD: >>= operator
    template<typename F>
    constexpr auto operator>>=(F f) const {
        return bind(f);
    }

    // FUNCTOR: fmap AU COMPILE-TIME
    template<typename F>
    constexpr auto fmap(F f) const {
        using R = decltype(f(value));
        if (!has_value) return Maybe<R>{};
        return Maybe<R>{f(value)};
    }
};

template<typename T>
constexpr Maybe<T> Just(T v) { return Maybe<T>{v}; }

constexpr Maybe<int> Nothing() { return Maybe<int>{}; }

// ============================================================================
// CONSTEXPR EITHER - ERROR HANDLING AU COMPILE-TIME !!!
// ============================================================================

template<typename E, typename A>
struct Either {
    A value;
    E error;
    bool is_right;

    constexpr Either(A v) : value(v), error{}, is_right(true) {}
    constexpr Either(E e, bool) : value{}, error(e), is_right(false) {}

    constexpr bool isRight() const { return is_right; }
    constexpr bool isLeft() const { return !is_right; }

    // MONAD: bind AU COMPILE-TIME
    template<typename F>
    constexpr auto bind(F f) const {
        using R = decltype(f(value));
        if (!is_right) return R{error, false};
        return f(value);
    }

    // FUNCTOR: fmap AU COMPILE-TIME
    template<typename F>
    constexpr auto fmap(F f) const {
        using B = decltype(f(value));
        if (!is_right) return Either<E, B>{error, false};
        return Either<E, B>{f(value)};
    }
};

template<typename E, typename A>
constexpr Either<E, A> Right(A v) { return Either<E, A>{v}; }

template<typename E, typename A>
constexpr Either<E, A> Left(E e) { return Either<E, A>{e, false}; }

// ============================================================================
// CONSTEXPR LIST - OPERATIONS AU COMPILE-TIME !!!
// ============================================================================

template<typename T, std::size_t N>
struct List {
    T data[N];
    std::size_t size;

    constexpr List() : data{}, size(0) {}

    constexpr void push(T val) { if (size < N) data[size++] = val; }
    constexpr T get(std::size_t i) const { return data[i]; }
    constexpr std::size_t length() const { return size; }

    // FUNCTOR: map AU COMPILE-TIME
    template<typename F>
    constexpr auto map(F f) const {
        using R = decltype(f(data[0]));
        List<R, N> result{};
        for (std::size_t i = 0; i < size; ++i)
            result.push(f(data[i]));
        return result;
    }

    // FILTER AU COMPILE-TIME
    template<typename F>
    constexpr auto filter(F pred) const {
        List<T, N> result{};
        for (std::size_t i = 0; i < size; ++i)
            if (pred(data[i]))
                result.push(data[i]);
        return result;
    }

    // FOLD AU COMPILE-TIME
    template<typename B, typename F>
    constexpr B fold(B init, F f) const {
        B acc = init;
        for (std::size_t i = 0; i < size; ++i)
            acc = f(acc, data[i]);
        return acc;
    }
};

// ============================================================================
// COMPILE-TIME MONAD CHAINS - TOUT RÉSOLU AVANT RUNTIME !!!
// ============================================================================

// Exemple: Safe division AU COMPILE-TIME
constexpr auto safeDivide(int a, int b) {
    if (b == 0)
        return Maybe<int>{};
    return Maybe<int>{a / b};
}

// Chain de computations AU COMPILE-TIME
constexpr auto computation() {
    return Just(100)
        .bind([](int x) { return safeDivide(x, 10); })
        .bind([](int x) { return safeDivide(x, 2); })
        .bind([](int x) { return Just(x * 3); });
}

// RÉSOLU AU COMPILE-TIME !!!
static_assert(computation().fromJust() == 15);  // (100 / 10) / 2 * 3 = 15

// ============================================================================
// PATTERN MATCHING AU COMPILE-TIME
// ============================================================================

template<typename T>
constexpr auto match_maybe(Maybe<T> m, auto just_case, auto nothing_case) {
    if (m.isJust())
        return just_case(m.fromJust());
    else
        return nothing_case();
}

// ============================================================================
// EXEMPLES ULTRA-PUISSANTS
// ============================================================================

// 1. Pipeline AU COMPILE-TIME
constexpr auto pipeline() {
    constexpr List<int, 10> nums{};
    constexpr auto init = [&]() {
        List<int, 10> l{};
        for (int i = 1; i <= 10; ++i) l.push(i);
        return l;
    }();

    return init
        .map([](int x) { return x * x; })
        .filter([](int x) { return x % 2 == 0; })
        .fold(0, [](int acc, int x) { return acc + x; });
}

static_assert(pipeline() == 220);  // 4 + 16 + 36 + 64 + 100

// Simplified - errorPipeline removed due to template complexity

// 3. Fibonacci avec Maybe AU COMPILE-TIME
constexpr auto fib_maybe(int n) -> Maybe<int> {
    if (n < 0) return Nothing();
    if (n <= 1) return Just(n);

    auto a = fib_maybe(n - 1);
    auto b = fib_maybe(n - 2);

    return a.bind([b](int x) {
        return b.fmap([x](int y) { return x + y; });
    });
}

static_assert(fib_maybe(10).fromJust() == 55);

} // namespace fp20::constexpr_monads
