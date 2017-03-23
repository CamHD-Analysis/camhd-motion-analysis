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
      : _start( start ), _end( end )
      {;}

    const T &start() const { return _start; }
    const T &end() const {return _end; }

    void reset( const T &start, const T &end )
      { _start = start; _end = end; }

    T span() { return _end - _start; }

    bool doBisect() {
      return _start != _end;
    }

    Interval<T> bisect( ) {
      auto split = middle( start(), end() );
      if( split == start() || split == end() ) return *this;

      auto before = Interval<T>( start(), split );
      reset( split, end() );
      return before;
    }

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


  // template <typename T>
  // T middle( const T &a, const T &b ) { return (a+b)/2; }


  template < typename T >
  class Intervals {
  public:

    typedef Interval<T> Interval;
    typedef std::vector<Interval> IntervalVec;

    Intervals()
      : _list()
      {;}

      void add( const T &a, const T &b );

      size_t size() const { return _list.size(); }

      const Interval &operator[](size_t i) const {
        return _list[i];
      }

      typename IntervalVec::const_iterator begin() const { return _list.begin(); }
      typename IntervalVec::const_iterator end() const { return _list.end(); }

      void bisect( std::function<bool(T,T)> plant, int maxDepth = -1, int depth = 0 );


      void dump();

  protected:

      IntervalVec _list;
  };

  template < typename T >
  ostream& operator<<(ostream& os, const Intervals<T>& a)
  {
    os << "List has " << a.size() << " elements:" << endl;
    for( auto const &i : a ) {
      os << "  " << i << endl;
    }
    return os;
  }

}

#include "interval.inl"
