
#include <gtest/gtest.h>

#include "camhd_client.h"
using namespace CamHDMotionTracking;

#include "test_data.h"

using namespace std;

TEST(test_camhd_client, test_get_movie) {
  auto movie( CamHDClient::getMovie( TestJsonLazycache ) );
}

TEST(test_camhd_client, test_make_frame_url) {
  auto movie( CamHDClient::getMovie( TestJsonLazycache ) );

  ASSERT_GT( movie.duration(), 0.0 );
  ASSERT_GT( movie.numFrames(), 0 );

  const int frameNum = 1000;
  fs::path frameUrl( CamHDClient::makeFrameURL( movie, frameNum ));

  std::stringstream urlStr;
  urlStr << TestJsonLazycache.string() << "/frame/" << frameNum;

  cout << "frameUrl: " << frameUrl << endl;
  cout << "url: " << urlStr.str() << endl;
  ASSERT_EQ( frameUrl.string(), urlStr.str() );
}


TEST(test_camhd_client, test_get_Frame) {
  auto movie( CamHDClient::getMovie( TestJsonLazycache ) );

  ASSERT_GT( movie.duration(), 0.0 );
  ASSERT_GT( movie.numFrames(), 0 );

  const int frameNum = 1000;
  cv::Mat frame( CamHDClient::getFrame( movie, frameNum ));

  ASSERT_EQ( frame.rows, 1080 );
  ASSERT_EQ( frame.cols, 1920 );
}
