#pragma once

#include <string>

#include <opencv2/core.hpp>

#include "json.hpp"
using json = nlohmann::json;

namespace CamHDMotionTracking {
  using std::string;

  class CamHDMovie {
  public:
    CamHDMovie();
    CamHDMovie( const string &url, const string &json = string("") );

    // void metadataFromJson( const string &json );

    const string &cacheUrl() const { return _cacheUrl; }
    const string &originalUrl() const { return _originalUrl; }

    float duration() const { return _duration; }
    int   numFrames() const { return _numFrames; }

    cv::Mat getFrame( int frame );

  protected:
      string _cacheUrl, _originalUrl;
      float _duration;
      int _numFrames;

      friend void from_json(const json& j, CamHDMovie& p);
  };

  inline void to_json(json& j, const CamHDMovie& mov) {
      j["cacheURL"] = mov.cacheUrl();
      j["URL"] = mov.originalUrl();
      j["Duration"] = mov.duration();
      j["NumFrames"] = mov.numFrames();
  }

  inline void from_json(const json& j, CamHDMovie& p) {
      p._originalUrl = j["URL"];
      p._duration = j["Duration"];
      p._numFrames = j["NumFrames"];
  }

}
