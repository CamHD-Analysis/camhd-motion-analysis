#pragma once

#include <string>

#include <opencv2/core.hpp>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

namespace CamHDMotionTracking {

using std::string;


class CamHDMovie {
public:
  CamHDMovie( const string &path, const string &json = "" );

protected:
    string _path;
};



class CamHDClient {
public:

  CamHDClient( const string &baseURL );

  CamHDMovie getMovie( const string &path );
  cv::Mat getFrame( const CamHDMovie &movie, int frame );

protected:

  fs::path _baseURL;

};

}
