
#include <gtest/gtest.h>

#include "camhd_movie.h"
using namespace CamHDMotionTracking;


#include "test_data.h"


TEST(test_camhd_movie, test_synchronous) {
  CamHDMovie movie( TestJsonUrl.string(), TestJson );

  ASSERT_FLOAT_EQ( movie.duration(), TestJsonDuration );
  ASSERT_EQ( movie.duration(), TestJsonDuration );
  ASSERT_EQ( movie.url(), TestJsonUrl );
}
