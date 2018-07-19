
#include <gtest/gtest.h>

#include <math.h>

#include "interval.h"

using namespace CamHDMotionTracking;




TEST(test_interval, test_constructor) {
  const int start = 1, end = 100;
  Interval<int> a( start, end );

  ASSERT_EQ( a.start(), start );
  ASSERT_EQ( a.end(), end );
}


TEST(test_intervals, test_constructor) {
  Intervals< int > list;
  ASSERT_EQ( list.size(), 0 );

  const int start = 1, end = 10;
  list.add( start, end );
  ASSERT_EQ( list.size(), 1);

  // ASSERT_EQ( list[0].start(), start );
  // ASSERT_EQ( list[0].end(), end );

}


float withinThree( int i, int j )
{ return abs(i-j) < 3.0; }

void testWithinThree( const Intervals<int> &list ) {
  for( auto interval : list ) {
    ASSERT_TRUE( withinThree( interval.start(), interval.end() ));
  }
}

TEST(test_intervals, test_bisect) {
  Intervals< int > list;

  const int start = 1, end = 10;
  list.add( start, end );

  list.bisect(withinThree);

testWithinThree( list );
}

TEST(test_intervals, test_bisect_zero_depth) {
  Intervals< int > list;

  const int start = 1, end = 10;
  list.add( start, end );

  list.bisect(withinThree, 0 );

  ASSERT_EQ( list.size(), 1 );
}

TEST(test_intervals, test_bisect_one_depth) {
  Intervals< int > list;

  const int start = 1, end = 10;
  list.add( start, end );

  list.bisect(withinThree, 1 );

  ASSERT_EQ( list.size(), 2 );
}
