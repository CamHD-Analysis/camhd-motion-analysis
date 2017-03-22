#pragma once

#include <string>

#include <opencv2/core.hpp>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "camhd_movie.h"

namespace CamHDMotionTracking {

using std::string;

class CamHDClient {
public:

  CamHDClient( ) = delete;
  CamHDClient( const CamHDClient & ) = delete;


  static CamHDMovie getMovie( const fs::path &path );
  static cv::Mat getFrame( const CamHDMovie &movie, int frame );

  static fs::path makeFrameURL( const CamHDMovie &mov, int frame );

protected:



  //fs::path _baseURL;

};

}
