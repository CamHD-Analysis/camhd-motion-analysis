
#include <gtest/gtest.h>

#include "camhd_movie.h"
using namespace CamHDMotionTracking;


#include "test_data.h"


TEST(test_camhd_movie, test_constructor_parse_json) {
  CamHDMovie movie( TestJsonLazycache.string(), TestJson );

  ASSERT_FLOAT_EQ( movie.duration(), TestJsonDuration );
  ASSERT_EQ( movie.duration(), TestJsonDuration );
  ASSERT_EQ( movie.cacheUrl(), TestJsonLazycache );
  ASSERT_EQ( movie.originalUrl(), TestJsonUrl );
}
