
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
    std::sort( _list.begin(), _list.end() );
  }

  template < typename T >
  void Intervals<T>::bisect( std::function<bool(T,T)> plant, int maxDepth, int depth )
 {
   if( depth > maxDepth ) {
     cout << "At max depth " << depth << ", stopping" << endl;
     return;
   }

    // First, determine if the interval needs to be bisected.
    for( auto i = _list.begin(); i < _list.end(); ++i ) {
      if( !plant( (*i).start(), (*i).end() ) ) {
        cout << "Bisect the set " << (*i) << "!" << endl;
      } else {
        cout << "Not bisecting " << (*i) << endl;
      }
    //   if( (*i).doBisect() ) {
    //     auto before = bisectInterval( *i );
    //     if( before == (*i) ) continue;
    //     _list.insert( i, before );
    //   }
    }
  }

  template < typename I >
  void Intervals<I>::dump() {
    cout << "List has " << _list.size() << " elements:" << endl;
    for( auto i : _list ) {
      cout << "  " << i << endl;
    }
  }

}
