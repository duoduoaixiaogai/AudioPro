#ifndef MEMORYX_H
#define MEMORYX_H

#include <memory>

namespace RF {
    template <typename T>
    struct Destroyer {
       void operator () (T *p) const { if (p) p->deleteLater(); }
    };

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
}

#endif
