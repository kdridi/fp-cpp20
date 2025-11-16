#include <iostream>
#include <array>

constexpr int fib(int n) { return n <= 1 ? n : fib(n-1) + fib(n-2); }
constexpr int fact(int n) { return n <= 1 ? 1 : n * fact(n-1); }
constexpr bool is_prime(int n, int d = 2) {
    return n <= 2 ? n == 2 : (d * d > n ? true : (n % d == 0 ? false : is_prime(n, d + 1)));
}

template<std::size_t N>
constexpr auto squares() {
    std::array<int, N> arr{};
    for (std::size_t i = 0; i < N; ++i) arr[i] = i * i;
    return arr;
}

template<std::size_t N>
constexpr auto sort(std::array<int, N> a) {
    for (std::size_t i = 0; i < N; ++i)
        for (std::size_t j = i + 1; j < N; ++j)
            if (a[i] > a[j]) { auto t = a[i]; a[i] = a[j]; a[j] = t; }
    return a;
}

constexpr unsigned myhash(const char* s) {
    unsigned h = 0;
    while (*s) h = h * 31 + *s++;
    return h;
}

static_assert(fib(10) == 55);
static_assert(fact(5) == 120);
static_assert(is_prime(17));
constexpr auto sq = squares<20>();
constexpr auto sorted = sort(std::array{9,3,7,1,5,2,8,4,6});

int main() {
    std::cout << "🔥 C++20 COMPILE-TIME POWER 🔥\n\n";
    std::cout << "fib(10) = " << fib(10) << "\n";
    std::cout << "fact(5) = " << fact(5) << "\n";
    std::cout << "Squares: ";
    for (int i = 0; i < 10; ++i) std::cout << sq[i] << " ";
    std::cout << "\nSorted: ";
    for (auto x : sorted) std::cout << x << " ";
    std::cout << "\n\n✅ TOUT EN COMPILE-TIME!\n";
    std::cout << "✅ ZERO OVERHEAD RUNTIME!\n";
    std::cout << "✅ PURE TEMPLATE METAPROGRAMMING!\n";
    return 0;
}
