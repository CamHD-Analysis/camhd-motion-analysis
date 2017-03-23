#pragma once

#include <vector>
#include <algorithm>
#include <iostream>

namespace CamHDMotionTracking {

  using std::vector;
  using std::ostream;
  using std::cout;
  using std::endl;

  template <typename T>
  class Interval {
  public:

    Interval( const T &start, const T &end )
      : _start(start), _end(end)
      {;}

    const T &start() const { return _start; }
    const T &end() const {return _end; }

    bool doBisect();

  protected:

    T _start, _end;

  };

  template <typename T>
  bool operator<( const Interval<T> &a, const Interval<T> &b )
    { return a.start() < b.start(); }

  template <typename T>
  ostream& operator<<(ostream& os, const Interval<T>& a) {
      os << a.start() << " -- " << a.end();
      return os;
  }

template <typename T>
bool equivalent( const T &a, const T &b ) { return a==b; }


  template < typename E, typename I = Interval<E> >
  class Intervals {
  public:

    Intervals()
      : _list()
      {;}

      void add( const E &start, const E &end );

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

  template < typename E, typename I = Interval<E> >
  ostream& operator<<(ostream& os, const Intervals<E,I>& a)
  {
    os << "List has " << a.size() << " elements:" << endl;
    for( auto const &i : a ) {
      os << "  " << i << endl;
    }
    return os;
  }

}

#include "interval.inl"
