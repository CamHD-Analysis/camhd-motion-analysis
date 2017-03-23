
#include <gtest/gtest.h>

#include "Interval.h"

using namespace CamHDMotionTracking;


TEST(test_interval, test_constructor) {
  const int start = 1, end = 100;
  Interval<int> a( start, end );

  ASSERT_EQ( a.start(), start );
  ASSERT_EQ( a.end(), end );
}

TEST(test_intervals, test_constructor) {
  Intervals<int> list;
  ASSERT_EQ( list.size(), 0 );

  const int start = 1, end = 10;
  list.add( start, end );
  ASSERT_EQ( list.size(), 1);

  ASSERT_EQ( list[0].start(), start );
  ASSERT_EQ( list[0].end(), end );

}
