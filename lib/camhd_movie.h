#pragma once

#include <string>

#include <opencv2/core/core.hpp>

#include "json.hpp"
using json = nlohmann::json;

namespace CamHDMotionTracking {
  using std::string;

  class CamHDMovie {
  public:
    CamHDMovie();
    CamHDMovie( const CamHDMovie &other );
    CamHDMovie( const string &url, const string &json = string("") );

    // void metadataFromJson( const string &json );

    bool initialized() const { return _initialized; };

    const string &cacheUrl() const { return _cacheUrl; }
    const string &originalUrl() const { return _originalUrl; }

    float duration() const { return _duration; }
    int   numFrames() const { return _numFrames; }

    cv::Mat getFrame( int frame );

  protected:
      string _cacheUrl, _originalUrl;
      float _duration;
      int _numFrames;
      bool _initialized;

      friend void from_json(const json& j, CamHDMovie& p);
  };

  inline void to_json(json& j, const CamHDMovie& mov) {
      j["cacheURL"] = mov.cacheUrl();
      j["URL"] = mov.originalUrl();
      j["Duration"] = mov.duration();
      j["NumFrames"] = mov.numFrames();
  }

  inline void from_json(const json& j, CamHDMovie& p) {
      if(j.find("URL") != j.end() ) p._originalUrl      = j["URL"];
      if(j.find("cacheURL") != j.end() ) p._cacheUrl    = j["cacheURL"];
      if(j.find("Duration") != j.end() ) p._duration    = j["Duration"];
      if(j.find("NumFrames") != j.end() ) p._numFrames   = j["NumFrames"];

      p._initialized = true;
  }

}
