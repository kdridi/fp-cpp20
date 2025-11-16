#include <fp20/monads/st.hpp>
#include <fp20/concepts/st_concepts.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace fp20;

static_assert(concepts::Functor<ST<int, int>>);
static_assert(concepts::Applicative<ST<int, int>>);
static_assert(concepts::Monad<ST<int, int>>);

namespace st_tests {

TEST_CASE("ST - Basic", "[st]") {
    auto st = ST<int, int>::pure(42);
    REQUIRE(st.evalST(0) == 42);

    auto st2 = st.fmap<int>([](int x) { return x * 2; });
    REQUIRE(st2.evalST(0) == 84);

    auto st3 = st.bind<int>([](int x) { return ST<int, int>::pure(x + 10); });
    REQUIRE(st3.evalST(0) == 52);
}

TEST_CASE("ST - Laws", "[st][laws]") {
    auto f = [](int x) { return ST<int, int>::pure(x + 1); };
    auto m = ST<int, int>::pure(42);
    REQUIRE(m.bind(f).evalST(0) == f(42).evalST(0));
}

}
