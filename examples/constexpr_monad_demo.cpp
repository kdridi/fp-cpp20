#include <iostream>
#include "../include/fp20/constexpr_monads.hpp"

using namespace fp20::constexpr_monads;

constexpr auto result1 = computation();
constexpr auto result2 = pipeline();
constexpr auto result4 = fib_maybe(10);

static_assert(result1.fromJust() == 15);
static_assert(result2 == 220);
static_assert(result4.fromJust() == 55);

int main() {
    std::cout << "🔥🔥🔥 CONSTEXPR MONADS - COMPILE-TIME MAGIC 🔥🔥🔥\n\n";
    std::cout << "Toutes ces valeurs calculées au COMPILE-TIME:\n\n";
    std::cout << "1. Monad chain: " << result1.fromJust() << " ✓\n";
    std::cout << "2. List pipeline: " << result2 << " ✓\n";
    std::cout << "3. Fibonacci(10): " << result4.fromJust() << " ✓\n\n";
    
    std::cout << "🎯 RÉVÉLATION:\n\n";
    std::cout << "✅ Monads runtime (flexibilité)\n";
    std::cout << "✅ Monads compile-time (ZERO overhead)\n";
    std::cout << "✅ Template metaprogramming (DSL power)\n\n";
    
    std::cout << "= HASKELL + C++ TEMPLATES = DIEU MODE 🚀💪⚡\n\n";
    std::cout << "Notre lib n'est PAS à drop!\n";
    std::cout << "Elle est maintenant TRANSCENDÉE!\n";
    
    return 0;
}
