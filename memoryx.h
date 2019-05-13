#ifndef MEMORYX_H
#define MEMORYX_H

#include <memory>
#include <functional>
//#include <cstdlib>

#include <algorithm>

namespace RF {
    template <typename T>
    struct Destroyer {
       void operator () (T *p) const { if (p) p->deleteLater(); }
    };

    template <typename T>
    using Destroy_ptr = std::unique_ptr<T, Destroyer<T>>;

    template<typename X>
    class ArrayOf : public std::unique_ptr<X[]>
    {
    public:
       ArrayOf() {}

       template<typename Integral>
       explicit ArrayOf(Integral count, bool initialize = false)
       {
          static_assert(std::is_unsigned<Integral>::value, "Unsigned arguments only");
          reinit(count, initialize);
       }

       //ArrayOf(const ArrayOf&) PROHIBITED;
       ArrayOf(const ArrayOf&) = delete;
       ArrayOf(ArrayOf&& that)
          : std::unique_ptr < X[] >
             (std::move((std::unique_ptr < X[] >&)(that)))
       {
       }
       ArrayOf& operator= (ArrayOf &&that)
       {
          std::unique_ptr<X[]>::operator=(std::move(that));
          return *this;
       }
       ArrayOf& operator= (std::unique_ptr<X[]> &&that)
       {
          std::unique_ptr<X[]>::operator=(std::move(that));
          return *this;
       }

       template< typename Integral >
       void reinit(Integral count,
                   bool initialize = false)
       {
          static_assert(std::is_unsigned<Integral>::value, "Unsigned arguments only");
          if (initialize)
             // Initialize elements (usually, to zero for a numerical type)
             std::unique_ptr<X[]>::reset(new X[count]{});
          else
             // Avoid the slight initialization overhead
             std::unique_ptr<X[]>::reset(new X[count]);
       }
    };

    template<typename X>
    class ArraysOf : public ArrayOf<ArrayOf<X>>
    {
    public:
       ArraysOf() {}

       template<typename Integral>
       explicit ArraysOf(Integral N)
          : ArrayOf<ArrayOf<X>>( N )
       {}

       template<typename Integral1, typename Integral2 >
       ArraysOf(Integral1 N, Integral2 M, bool initialize = false)
       : ArrayOf<ArrayOf<X>>( N )
       {
          static_assert(std::is_unsigned<Integral1>::value, "Unsigned arguments only");
          static_assert(std::is_unsigned<Integral2>::value, "Unsigned arguments only");
          for (size_t ii = 0; ii < N; ++ii)
             (*this)[ii] = ArrayOf<X>{ M, initialize };
       }

       //ArraysOf(const ArraysOf&) PROHIBITED;
       ArraysOf(const ArraysOf&) =delete;
       ArraysOf& operator= (ArraysOf&& that)
       {
          ArrayOf<ArrayOf<X>>::operator=(std::move(that));
          return *this;
       }

       template< typename Integral >
       void reinit(Integral count)
       {
          ArrayOf<ArrayOf<X>>::reinit( count );
       }

       template< typename Integral >
       void reinit(Integral count, bool initialize)
       {
          ArrayOf<ArrayOf<X>>::reinit( count, initialize );
       }

       template<typename Integral1, typename Integral2 >
       void reinit(Integral1 countN, Integral2 countM, bool initialize = false)
       {
          static_assert(std::is_unsigned<Integral1>::value, "Unsigned arguments only");
          static_assert(std::is_unsigned<Integral2>::value, "Unsigned arguments only");
          reinit(countN, false);
          for (size_t ii = 0; ii < countN; ++ii)
             (*this)[ii].reinit(countM, initialize);
       }
    };

    template <typename F>
    struct Final_action {
       Final_action(F f) : clean( f ) {}
       ~Final_action() { clean(); }
       F clean;
    };

    template <typename F>
    Final_action<F> finally (F f)
    {
       return Final_action<F>(f);
    }

    template <typename Iterator>
    struct IteratorRange : public std::pair<Iterator, Iterator> {
       using iterator = Iterator;
       using reverse_iterator = std::reverse_iterator<Iterator>;

       IteratorRange (const Iterator &a, const Iterator &b)
       : std::pair<Iterator, Iterator> ( a, b ) {}

       IteratorRange (Iterator &&a, Iterator &&b)
       : std::pair<Iterator, Iterator> ( std::move(a), std::move(b) ) {}

       IteratorRange< reverse_iterator > reversal () const
       { return { this->rbegin(), this->rend() }; }

       Iterator begin() const { return this->first; }
       Iterator end() const { return this->second; }

       reverse_iterator rbegin() const { return reverse_iterator{ this->second }; }
       reverse_iterator rend() const { return reverse_iterator{ this->first }; }

       bool empty() const { return this->begin() == this->end(); }
       explicit operator bool () const { return !this->empty(); }
       size_t size() const { return std::distance(this->begin(), this->end()); }

       template <typename T> iterator find(const T &t) const
       { return std::find(this->begin(), this->end(), t); }

       template <typename T> long index(const T &t) const
       {
          auto iter = this->find(t);
          if (iter == this->end())
             return -1;
          return std::distance(this->begin(), iter);
       }

       template <typename T> bool contains(const T &t) const
       { return this->end() != this->find(t); }

       template <typename F> iterator find_if(const F &f) const
       { return std::find_if(this->begin(), this->end(), f); }

       template <typename F> long index_if(const F &f) const
       {
          auto iter = this->find_if(f);
          if (iter == this->end())
             return -1;
          return std::distance(this->begin(), iter);
       }

       // to do: use std::all_of, any_of, none_of when available on all platforms
       template <typename F> bool all_of(const F &f) const
       {
          auto notF =
             [&](typename std::iterator_traits<Iterator>::reference v)
                { return !f(v); };
          return !this->any_of( notF );
       }

       template <typename F> bool any_of(const F &f) const
       { return this->end() != this->find_if(f); }

       template <typename F> bool none_of(const F &f) const
       { return !this->any_of(f); }

       template<typename T> struct identity
          { const T&& operator () (T &&v) const { return std::forward(v); } };

       // Like std::accumulate, but the iterators implied, and with another
       // unary operation on the iterator value, pre-composed
       template<
          typename R,
          typename Binary = std::plus< R >,
          typename Unary = identity< decltype( *std::declval<Iterator>() ) >
       >
       R accumulate(
          R init,
          Binary binary_op = {},
          Unary unary_op = {}
       ) const
       {
          R result = init;
          for (auto&& v : *this)
             result = binary_op(result, unary_op(v));
          return result;
       }

       // An overload making it more convenient to use with pointers to member
       // functions
       template<
          typename R,
          typename Binary = std::plus< R >,
          typename R2, typename C
       >
       R accumulate(
          R init,
          Binary binary_op,
          R2 (C :: * pmf) () const
       ) const
       {
          return this->accumulate( init, binary_op, std::mem_fn( pmf ) );
       }

       // Some accumulations frequent enough to be worth abbreviation:
       template<
          typename Unary = identity< decltype( *std::declval<Iterator>() ) >,
          typename R = decltype( std::declval<Unary>()( *std::declval<Iterator>() ) )
       >
       R min( Unary unary_op = {} ) const
       {
          return this->accumulate(
             std::numeric_limits< R >::max(),
             (const R&(*)(const R&, const R&)) std::min,
             unary_op
          );
       }

       template<
          typename R2, typename C,
          typename R = R2
       >
       R min( R2 (C :: * pmf) () const ) const
       {
          return this->min( std::mem_fn( pmf ) );
       }

       template<
          typename Unary = identity< decltype( *std::declval<Iterator>() ) >,
          typename R = decltype( std::declval<Unary>()( *std::declval<Iterator>() ) )
       >
       R max( Unary unary_op = {} ) const
       {
          return this->accumulate(
             std::numeric_limits< R >::lowest(),
             (const R&(*)(const R&, const R&)) std::max,
             unary_op
          );
       }

       template<
          typename R2, typename C,
          typename R = R2
       >
       R max( R2 (C :: * pmf) () const ) const
       {
          return this->max( std::mem_fn( pmf ) );
       }

       template<
          typename Unary = identity< decltype( *std::declval<Iterator>() ) >,
          typename R = decltype( std::declval<Unary>()( *std::declval<Iterator>() ) )
       >
       R sum( Unary unary_op = {} ) const
       {
          return this->accumulate(
             R{ 0 },
             std::plus< R >{},
             unary_op
          );
       }

       template<
          typename R2, typename C,
          typename R = R2
       >
       R sum( R2 (C :: * pmf) () const ) const
       {
          return this->sum( std::mem_fn( pmf ) );
       }
    };
}

#endif
