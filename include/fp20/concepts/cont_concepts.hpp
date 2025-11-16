// ============================================================================
// FP++20: Continuation Monad Concepts
// ============================================================================
// Concepts for compile-time verification of Cont<R, A> monad instances
// and continuation-aware computations.
//
// ============================================================================
// ACADEMIC REFERENCES
// ============================================================================
// - C++20 Concepts: https://en.cppreference.com/w/cpp/language/constraints
// - Type Classes in Haskell: https://wiki.haskell.org/Type_class
// - Cont Monad Laws: https://wiki.haskell.org/Continuation
//
// ============================================================================

#pragma once

#include <concepts>
#include <functional>
#include <type_traits>

namespace fp20 {

    // Forward declare Cont template
    template<typename R, typename A>
    class Cont;

    // ========================================================================
    // CONT INSTANCE CONCEPT
    // ========================================================================
    // Verifies that a type is a Cont<R, A> instance with proper structure

    template<typename T>
    concept ContInstance = requires {
        // Must have result_type and value_type
        typename T::result_type;
        typename T::value_type;
    } && requires(const T& cont, std::function<typename T::result_type(typename T::value_type)> k) {
        // Must have runCont method with correct signature
        { cont.runCont(k) } -> std::same_as<typename T::result_type>;
    };

    // Verify that Cont template satisfies ContInstance
    static_assert(ContInstance<Cont<int, int>>);
    static_assert(ContInstance<Cont<std::string, int>>);
    static_assert(ContInstance<Cont<bool, std::string>>);

    // ========================================================================
    // FUNCTOR CONCEPT FOR CONT
    // ========================================================================
    // Verifies that fmap exists and works correctly for Cont

    template<typename M>
    concept Functor = ContInstance<M> && requires(
        const M& m,
        std::function<int(typename M::value_type)> f
    ) {
        // fmap must exist and preserve result_type
        { fmap(f, m) } -> ContInstance;
        requires std::same_as<
            typename decltype(fmap(f, m))::result_type,
            typename M::result_type
        >;
    };

    // Verify Functor for various Cont instantiations
    static_assert(Functor<Cont<int, std::string>>);
    static_assert(Functor<Cont<bool, int>>);

    // ========================================================================
    // APPLICATIVE CONCEPT FOR CONT
    // ========================================================================
    // Verifies that pure and apply exist and work correctly

    template<typename M>
    concept Applicative = Functor<M> && requires(typename M::value_type a) {
        // pure must exist and create proper Cont
        { pure<typename M::result_type>(a) } -> ContInstance;
        requires std::same_as<
            decltype(pure<typename M::result_type>(a)),
            Cont<typename M::result_type, typename M::value_type>
        >;
    };

    // Verify Applicative for various Cont instantiations
    static_assert(Applicative<Cont<int, int>>);
    static_assert(Applicative<Cont<std::string, bool>>);

    // ========================================================================
    // MONAD CONCEPT FOR CONT
    // ========================================================================
    // Verifies that bind exists and works correctly

    template<typename M>
    concept Monad = Applicative<M> && requires(
        const M& m,
        std::function<Cont<typename M::result_type, int>(typename M::value_type)> k
    ) {
        // bind must exist and preserve result_type
        { bind(m, k) } -> ContInstance;
        requires std::same_as<
            typename decltype(bind(m, k))::result_type,
            typename M::result_type
        >;
    };

    // Verify Monad for various Cont instantiations
    static_assert(Monad<Cont<int, std::string>>);
    static_assert(Monad<Cont<bool, int>>);

    // ========================================================================
    // CONTINUATION COMPUTATION CONCEPT
    // ========================================================================
    // Verifies that a type can be used as a continuation computation

    template<typename F, typename A, typename R>
    concept ContinuationFunction = requires(F f, std::function<R(A)> k) {
        { f(k) } -> std::same_as<R>;
    };

    // ========================================================================
    // ESCAPE CONTINUATION CONCEPT (for callCC)
    // ========================================================================
    // Verifies that a function can be used as an escape continuation

    template<typename F, typename A, typename R, typename B>
    concept EscapeFunction = requires(F f, A a) {
        { f(a) } -> std::same_as<Cont<R, B>>;
    };

    // ========================================================================
    // CALLCC FUNCTION CONCEPT
    // ========================================================================
    // Verifies that a function can be used with callCC

    template<typename F, typename R, typename A, typename B>
    concept CallCCFunction = requires(F f, std::function<Cont<R, B>(A)> escape) {
        { f(escape) } -> std::same_as<Cont<R, A>>;
    };

    // ========================================================================
    // TYPE TRAIT HELPERS
    // ========================================================================

    // Extract result type from Cont
    template<typename T>
    struct cont_result_type;

    template<typename R, typename A>
    struct cont_result_type<Cont<R, A>> {
        using type = R;
    };

    template<typename T>
    using cont_result_type_t = typename cont_result_type<T>::type;

    // Extract value type from Cont
    template<typename T>
    struct cont_value_type;

    template<typename R, typename A>
    struct cont_value_type<Cont<R, A>> {
        using type = A;
    };

    template<typename T>
    using cont_value_type_t = typename cont_value_type<T>::type;

    // Check if a type is a Cont instance
    template<typename T>
    struct is_cont : std::false_type {};

    template<typename R, typename A>
    struct is_cont<Cont<R, A>> : std::true_type {};

    template<typename T>
    inline constexpr bool is_cont_v = is_cont<T>::value;

    // Verify type traits work
    static_assert(is_cont_v<Cont<int, std::string>>);
    static_assert(!is_cont_v<int>);
    static_assert(!is_cont_v<std::string>);
    static_assert(std::same_as<cont_result_type_t<Cont<int, std::string>>, int>);
    static_assert(std::same_as<cont_value_type_t<Cont<int, std::string>>, std::string>);

    // ========================================================================
    // CONCEPT VERIFICATION EXAMPLES
    // ========================================================================
    //
    // These static assertions verify that the concepts work correctly:
    //
    // 1. Cont instances satisfy ContInstance:
    //    static_assert(ContInstance<Cont<int, std::string>>);
    //
    // 2. Cont instances satisfy Functor:
    //    static_assert(Functor<Cont<int, std::string>>);
    //
    // 3. Cont instances satisfy Applicative:
    //    static_assert(Applicative<Cont<int, std::string>>);
    //
    // 4. Cont instances satisfy Monad:
    //    static_assert(Monad<Cont<int, std::string>>);
    //
    // 5. Type traits extract correct types:
    //    static_assert(std::same_as<cont_result_type_t<Cont<int, bool>>, int>);
    //    static_assert(std::same_as<cont_value_type_t<Cont<int, bool>>, bool>);
    //
    // ========================================================================

} // namespace fp20
