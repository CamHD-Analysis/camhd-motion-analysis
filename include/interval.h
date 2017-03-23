#pragma once

#include <list>
#include <algorithm>
#include <iostream>

namespace CamHDMotionTracking {

  using std::list;
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

    void setBounds( const T &start, const T &end )
      { _start = start; _end = end; }

    T span() { return _end - _start; }

    bool doBisect() {
      return _start != _end;
    }

    Interval<T> bisect( ) {
      T split = floor( 0.5 * (start() + end()) );
      if( split <= start() || split+1 >= end() ) return *this;

      auto before = Interval<T>( start(), split );
      setBounds( split+1, end() );
      return before;
    }

    bool operator==( const Interval<T> &other ) {
      return start() == other.start() && end() == other.end();
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
    typedef std::list<Interval> IntervalVec;

    Intervals()
      : _list()
      {;}

      void add( const T &a, const T &b );

      size_t size() const { return _list.size(); }

      // const Interval &operator[](size_t i) const {
      //   return _list[i];
      // }

      typename IntervalVec::const_iterator begin() const { return _list.begin(); }
      typename IntervalVec::const_iterator end() const { return _list.end(); }

      void bisect( std::function<bool(T,T)> plant, int maxDepth = -1 );


      void dump();

  protected:

    void doBisect( typename IntervalVec::iterator &i, std::function<bool(T,T)> plant, int maxDepth, int depth = 0 );

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
