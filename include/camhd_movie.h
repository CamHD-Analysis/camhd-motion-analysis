#pragma once

#include <string>

#include <opencv2/core.hpp>

namespace CamHDMotionTracking {
  using std::string;

  class CamHDMovie {
  public:
    CamHDMovie();
    CamHDMovie( const string &url, const string &json );

    void metadataFromJson( const string &json );

    const string &url() const { return _url; }
    const string &originalUrl() const { return _originalUrl; }

    float duration() const { return _duration; }
    int   numFrames() const { return _numFrames; }

    cv::Mat getFrame( int frame );


  protected:
      string _url, _originalUrl;
      float _duration;
      int _numFrames;
  };
}
