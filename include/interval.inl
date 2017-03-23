
#include <iostream>

namespace CamHDMotionTracking {

  using std::cout;
  using std::endl;

  //===================

  // template <typename X, typename Y>
  // bool NumericalInterval<X,Y>::doBisect() {
  //   return equivalent(_start,_end);
  // }


  //===================

  template < typename T >
  void Intervals<T>::add( const T &start, const T &end ) {
    // Save efficiency for later
    _list.push_back( Interval(start, end) );
    _list.sort( );
  }

  template < typename T >
  void Intervals<T>::bisect( std::function<bool(T,T)> plant, int maxDepth )
 {

    // First, determine if the interval needs to be bisected.
    for( auto i = _list.begin(); i != _list.end(); ++i ) {
      doBisect( i, plant, maxDepth );
    }
  }

  template < typename T >
  void Intervals<T>::doBisect( typename IntervalVec::iterator &i,
                                std::function<bool(T,T)> plant, int maxDepth, int depth ) {

    if( (maxDepth >= 0) && (depth >= maxDepth) ) {
      //cout << "At max depth " << depth << ", stopping" << endl;
      return;
    }

    if( !plant( (*i).start(), (*i).end() ) ) {
      //cout << "-- Bisect the set --: " << (*i) << "!" << endl;
      auto newInterval = (*i).bisect();

      // Check for non-bisection
      if (newInterval == *i) return;

      auto newItr = _list.insert( i, newInterval );

      doBisect( newItr, plant, maxDepth, depth+1 );
      doBisect( i, plant, maxDepth, depth+1 );

    } //else {
      //cout << "Not bisecting " << (*i) << endl;
    //}
  }

  template < typename I >
  void Intervals<I>::dump() {
    cout << "List has " << _list.size() << " elements:" << endl;
    for( auto i : _list ) {
      cout << "  " << i << endl;
    }
  }

}
