
#include <iostream>

namespace CamHDMotionTracking {

  using std::cout;
  using std::endl;

  template < typename E, typename I >
  void Intervals<E,I>::add( const E &start, const E &end ) {
    // Save efficiency for later
    _list.push_back( I(start, end) );
      std::sort( _list.begin(), _list.end() );
  }

  template < typename E, typename I >
  void Intervals<E,I>::bisect() {

  }

  template < typename E, typename I >
  void Intervals<E,I>::dump() {
    cout << "List has " << _list.size() << " elements:" << endl;
    for( auto i : _list ) {
      cout << "  " << i << endl;
    }
  }

}
