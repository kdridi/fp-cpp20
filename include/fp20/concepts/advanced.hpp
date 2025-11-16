#pragma once

#include <concepts>
#include <type_traits>

namespace fp20::concepts {

// ============================================================================
// CATEGORY THEORY - ULTRA HIGH LEVEL
// ============================================================================

// Semigroup: Associative binary operation
template<typename T>
concept Semigroup = requires(T a, T b) {
    { mappend(a, b) } -> std::same_as<T>;
};

// Monoid: Semigroup with identity element
template<typename T>
concept Monoid = Semigroup<T> && requires {
    { mempty<T>() } -> std::same_as<T>;
};

// Foldable: Can be folded/reduced
template<typename T>
concept Foldable = requires(T t) {
    { foldr([](auto x, auto acc) { return acc; }, 0, t) };
    { foldl([](auto acc, auto x) { return acc; }, 0, t) };
};

// Traversable: Can be traversed with effects
template<typename T>
concept Traversable = Foldable<T> && Functor<T>;

// Alternative: MonadPlus for Applicatives
template<typename F>
concept Alternative = Applicative<F> && requires(F f) {
    { empty<F>() };
    { alt(f, f) } -> std::same_as<F>;
};

// Arrow: Generalized functions
template<typename A>
concept Arrow = requires {
    // arr :: (b -> c) -> a b c
    // (>>>) :: a b c -> a c d -> a b d
    // first :: a b c -> a (b,d) (c,d)
};

// Comonad: Dual of Monad
template<typename W>
concept Comonad = Functor<W> && requires(W w) {
    // extract :: w a -> a
    { extract(w) };
    // extend :: (w a -> b) -> w a -> w b
    { extend([](W wa) { return extract(wa); }, w) } -> std::same_as<W>;
};

// Bifunctor: Functor in two arguments
template<typename F>
concept Bifunctor = requires {
    // bimap :: (a -> c) -> (b -> d) -> f a b -> f c d
};

// Profunctor: Contravariant in first arg, covariant in second
template<typename P>
concept Profunctor = requires {
    // dimap :: (a' -> a) -> (b -> b') -> p a b -> p a' b'
};

// ============================================================================
// OPTICS - LENS, PRISM, TRAVERSAL
// ============================================================================

// Lens: Composable getters/setters
template<typename S, typename A>
concept Lens = requires(S s) {
    // view :: Lens s a -> s -> a
    { view(s) } -> std::convertible_to<A>;
    // set :: Lens s a -> a -> s -> s
    { set(A{}, s) } -> std::same_as<S>;
};

// Prism: Optional constructor/destructor
template<typename S, typename A>
concept Prism = requires(S s) {
    // preview :: Prism s a -> s -> Maybe a
    // review :: Prism s a -> a -> s
};

// Iso: Isomorphism (convertible both ways)
template<typename S, typename A>
concept Iso = requires(S s, A a) {
    { from(s) } -> std::same_as<A>;
    { to(a) } -> std::same_as<S>;
};

// ============================================================================
// FREE STRUCTURES
// ============================================================================

// Free Monad: DSL construction
template<typename F>
concept FreeMonad = Monad<F> && requires {
    // Pure :: a -> Free f a
    // Free :: f (Free f a) -> Free f a
};

// Free Applicative: Applicative DSL
template<typename F>
concept FreeApplicative = Applicative<F>;

// Cofree Comonad: Dual of Free
template<typename F>
concept CofreeComonad = Comonad<F>;

// ============================================================================
// RECURSION SCHEMES - ULTRA HIGH LEVEL
// ============================================================================

// Functor for recursion schemes
template<typename F>
concept RecursiveFunctor = Functor<F>;

// Catamorphism: fold
template<typename F, typename A>
concept Cata = RecursiveFunctor<F> && requires(F f, A a) {
    { cata([](F fx) { return a; }, f) } -> std::same_as<A>;
};

// Anamorphism: unfold
template<typename F, typename A>
concept Ana = RecursiveFunctor<F> && requires(A a) {
    { ana([](A x) { return F{}; }, a) } -> std::same_as<F>;
};

// Hylomorphism: ana followed by cata
template<typename F, typename A, typename B>
concept Hylo = Ana<F, A> && Cata<F, B>;

// Paramorphism: primitive recursion
template<typename F>
concept Para = RecursiveFunctor<F>;

// Apomorphism: dual of para
template<typename F>
concept Apo = RecursiveFunctor<F>;

// ============================================================================
// MONAD TRANSFORMERS
// ============================================================================

// MonadTrans: Monad transformer
template<typename T>
concept MonadTrans = requires {
    // lift :: m a -> t m a
};

// MaybeT: Maybe monad transformer
template<typename M>
concept MaybeT = MonadTrans<M> && Monad<M>;

// EitherT: Either monad transformer
template<typename E, typename M>
concept EitherT = MonadTrans<M> && Monad<M>;

// StateT: State monad transformer
template<typename S, typename M>
concept StateT = MonadTrans<M> && Monad<M>;

// ReaderT: Reader monad transformer
template<typename R, typename M>
concept ReaderT = MonadTrans<M> && Monad<M>;

// WriterT: Writer monad transformer
template<typename W, typename M>
concept WriterT = MonadTrans<M> && Monad<M> && Monoid<W>;

// ============================================================================
// EFFECT SYSTEMS
// ============================================================================

// Eff: Extensible effects
template<typename E>
concept Effect = requires {
    // send :: eff -> Eff effs a
    // run :: Eff '[] a -> a
};

// MTL-style typeclasses
template<typename M>
concept MonadReader = Monad<M> && requires(M m) {
    { ask<M>() };
    { local([](auto r) { return r; }, m) } -> std::same_as<M>;
};

template<typename M>
concept MonadState = Monad<M> && requires {
    { get<M>() };
    { put<M>(0) };
    { modify<M>([](auto s) { return s; }) };
};

template<typename M>
concept MonadWriter = Monad<M> && requires {
    { tell<M>(0) };
    { listen<M>(M{}) };
};

template<typename M>
concept MonadError = Monad<M> && requires {
    { throwError<M>(0) };
    { catchError<M>(M{}, [](auto e) { return M{}; }) };
};

// ============================================================================
// CONSTRAINTS & REFINEMENT TYPES
// ============================================================================

// Refined: Value-level constraints
template<typename T, auto Predicate>
concept Refined = requires(T t) {
    requires Predicate(t);
};

// NonEmpty: Non-empty containers
template<typename C>
concept NonEmpty = requires(C c) {
    requires !c.empty();
    { head(c) };
    { tail(c) } -> std::same_as<C>;
};

// Positive: Positive numbers
template<typename T>
concept Positive = std::is_arithmetic_v<T> && requires(T t) {
    requires t > T{0};
};

// ============================================================================
// FIXPOINT & Y-COMBINATOR
// ============================================================================

// Fix: Fixpoint combinator
template<typename F>
concept FixPoint = requires(F f) {
    { fix(f) };
};

// Mu: Least fixpoint (inductive)
template<typename F>
concept Mu = RecursiveFunctor<F>;

// Nu: Greatest fixpoint (coinductive)
template<typename F>
concept Nu = RecursiveFunctor<F>;

} // namespace fp20::concepts
