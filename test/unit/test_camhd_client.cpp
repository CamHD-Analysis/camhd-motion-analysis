
#include <gtest/gtest.h>

#include "camhd_client.h"
using namespace CamHDMotionTracking;

#include "test_data.h"

using namespace std;

TEST(test_camhd_client, test_get_movie) {
  auto movie( CamHDClient::getMovie( TestJsonLazycache ) );

  ASSERT_EQ( movie.cacheUrl(), TestJsonLazycache.string() );
  ASSERT_EQ( movie.originalUrl(), TestJsonUrl.string() );
  ASSERT_EQ( movie.duration(), TestJsonDuration );
  ASSERT_EQ( movie.numFrames(), TestJsonNumFrames );
}

TEST(test_camhd_client, test_make_frame_url) {
  auto movie( CamHDClient::getMovie( TestJsonLazycache ) );

  const int frameNum = 1000;
  fs::path frameUrl( CamHDClient::makeFrameURL( movie, frameNum ));

  std::stringstream truthStr;
  truthStr << TestJsonLazycache.string() << "/frame/" << frameNum;

  // cout << "frameUrl: " << frameUrl << endl;
  // cout << "url: " << urlStr.str() << endl;
  ASSERT_EQ( frameUrl.string(), truthStr.str() );
}


TEST(test_camhd_client, test_get_frame) {
  auto movie( CamHDClient::getMovie( TestJsonLazycache ) );

  const int frameNum = 1000;
  cv::Mat frame( CamHDClient::getFrame( movie, frameNum ));

  ASSERT_EQ( frame.rows, 1080 );
  ASSERT_EQ( frame.cols, 1920 );
}

TEST(test_camhd_client, test_get_last_frame) {
  auto movie( CamHDClient::getMovie( TestJsonLazycache ) );

  ASSERT_GT( movie.duration(), 0.0 );
  ASSERT_GT( movie.numFrames(), 0 );

  cv::Mat frame( CamHDClient::getFrame( movie, movie.numFrames()+1 ));

  ASSERT_EQ( frame.rows, 1080 );
  ASSERT_EQ( frame.cols, 1920 );

  // Check that it's black
  auto m = cv::mean( frame );
  ASSERT_FLOAT_EQ( m[0], 0.0 );
  ASSERT_FLOAT_EQ( m[1], 0.0 );
  ASSERT_FLOAT_EQ( m[2], 0.0 );
  ASSERT_FLOAT_EQ( m[3], 0.0 );
}
