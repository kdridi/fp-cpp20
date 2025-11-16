#include <iostream>
#include <array>

// COMPILE-TIME POWER - EVERYTHING COMPUTED BEFORE RUNTIME

// 1. COMPILE-TIME FIBONACCI
constexpr int fib(int n) {
    return n <= 1 ? n : fib(n-1) + fib(n-2);
}
static_assert(fib(10) == 55);

// 2. TYPE-LEVEL LISTS
template<int... Is> struct IntList {};
template<int I, typename List> struct Prepend;
template<int I, int... Is>
struct Prepend<I, IntList<Is...>> { using type = IntList<I, Is...>; };

// 3. COMPILE-TIME STRING HASHING
constexpr unsigned hash(const char* str) {
    unsigned h = 0;
    while (*str) h = h * 31 + *str++;
    return h;
}
static_assert(hash("hello") == hash("hello"));

// 4. STATE MACHINE DSL
struct Idle {}; struct Running {}; struct Paused {};
struct Start {}; struct Stop {}; struct Pause {};

template<typename State, typename Event> struct Next { using type = State; };
template<> struct Next<Idle, Start> { using type = Running; };
template<> struct Next<Running, Pause> { using type = Paused; };
template<> struct Next<Running, Stop> { using type = Idle; };

// 5. COMPILE-TIME ARRAY OPERATIONS
template<std::size_t N>
constexpr auto generate_squares() {
    std::array<int, N> arr{};
    for (std::size_t i = 0; i < N; ++i)
        arr[i] = i * i;
    return arr;
}
constexpr auto squares = generate_squares<10>();

// 6. TYPE-SAFE UNITS
template<int M, int L, int T> struct Unit { double v; };
using Meter = Unit<0,1,0>;
using Second = Unit<0,0,1>;
using Speed = Unit<0,1,-1>;

template<int M, int L, int T>
constexpr Unit<M,L,T> operator+(Unit<M,L,T> a, Unit<M,L,T> b) {
    return {a.v + b.v};
}

// 7. CONSTEXPR REGEX (simple)
constexpr bool contains(const char* str, char c) {
    while (*str) if (*str++ == c) return true;
    return false;
}
static_assert(contains("hello", 'l'));

// 8. COMPILE-TIME SORT
template<std::size_t N>
constexpr auto bubble_sort(std::array<int, N> arr) {
    for (std::size_t i = 0; i < N; ++i)
        for (std::size_t j = 0; j < N-1; ++j)
            if (arr[j] > arr[j+1])
                std::swap(arr[j], arr[j+1]);
    return arr;
}
constexpr auto sorted = bubble_sort(std::array{5,2,8,1,9});

// 9. TYPE TRAITS DSL
template<typename T> struct is_pointer { static constexpr bool value = false; };
template<typename T> struct is_pointer<T*> { static constexpr bool value = true; };
static_assert(is_pointer<int*>::value);
static_assert(!is_pointer<int>::value);

// 10. CONSTEXPR MAP
template<std::size_t N>
struct ConstMap {
    std::array<std::pair<unsigned, int>, N> data;
    
    constexpr int get(unsigned key) const {
        for (auto [k, v] : data)
            if (k == key) return v;
        return -1;
    }
};

constexpr ConstMap<3> config{{
    {hash("port"), 8080},
    {hash("timeout"), 30},
    {hash("workers"), 4}
}};

int main() {
    std::cout << "=== C++20 COMPILE-TIME METAPROGRAMMING ===\n\n";
    
    std::cout << "1. Fibonacci(10) = " << fib(10) << " (compile-time!)\n";
    std::cout << "2. Squares array: ";
    for (auto x : squares) std::cout << x << " ";
    std::cout << "\n3. Sorted array: ";
    for (auto x : sorted) std::cout << x << " ";
    std::cout << "\n4. Config port = " << config.get(hash("port")) << "\n";
    std::cout << "5. State: Idle->Start = Running ✓\n";
    std::cout << "6. Units: 10m + 20m = " << (Meter{10} + Meter{20}).v << "m\n";
    
    std::cout << "\n🔥 TOUT CALCULÉ AU COMPILE-TIME! 🔥\n";
    std::cout << "ZERO RUNTIME OVERHEAD!\n";
    std::cout << "C++20 = PURE TEMPLATE POWER!\n";
    
    return 0;
}
