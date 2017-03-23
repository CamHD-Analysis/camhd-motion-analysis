#pragma once

#include <vector>
#include <algorithm>
#include <iostream>

namespace CamHDMotionTracking {

  using std::vector;
  using std::ostream;
  using std::cout;
  using std::endl;

  template <typename T, typename Y>
  class NumericalInterval {
  public:

    NumericalInterval( const X &start, const X &end, std::UnaryFunction<T,Y> f )
      : _start( start ), _end( end ), _func( f )
      {;}

    const T &start() const { return _start; }
    const T &end() const {return _end; }

    void reset( const T &start, const T &end )
      { _start = start; _end = end; }

    T span() { return _end - _start; }

    bool doBisect();

    T value( const Y &y ) { return _func(y); }

    NumericalInterval<T> bisect( ) {
      auto split = middle( start(), end() );
      if( split == start() || split == end() ) return *this;

      auto before = NumericalInterval<T>( start(), split );
      reset( split, end() );
      return before;
    }

  protected:

    T _start, _end;
    std::UnaryFunction _func;

  };

  template <typename T>
  bool operator<( const NumericalInterval<T> &a, const NumericalInterval<T> &b )
    { return a.start() < b.start(); }

  template <typename T>
  ostream& operator<<(ostream& os, const NumericalInterval<T>& a) {
      os << a.start() << " -- " << a.end();
      return os;
  }


  template <typename T>
  T middle( const T &a, const T &b ) { return (a+b)/2; }


  template < typename I = Interval<E> >
  class Intervals {
  public:

    Intervals()
      : _list()
      {;}

      void add( const I &interval );

      size_t size() const { return _list.size(); }

      const I &operator[](size_t i) const {
        return _list[i];
      }

      typename vector< I >::const_iterator begin() const { return _list.begin(); }
      typename vector< I >::const_iterator end() const { return _list.end(); }


      void bisect();
      void dump();

  protected:

      vector< I > _list;
  };

  template < typename I >
  ostream& operator<<(ostream& os, const Intervals<I>& a)
  {
    os << "List has " << a.size() << " elements:" << endl;
    for( auto const &i : a ) {
      os << "  " << i << endl;
    }
    return os;
  }

}

#include "interval.inl"
