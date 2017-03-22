#pragma once

#include <string>


namespace CamHDMotionTracking {
  using std::string;

  class CamHDMovie {
  public:
    CamHDMovie( const string &path, const string &json );

    void metadataFromJson( const string &json );

    const string &url() const { return _url; }
    const string &path() const { return _path; }

    float duration() const { return _duration; }
    int   numFrames() const { return _numFrames; }


  protected:
      string _path, _url;
      float _duration;
      int _numFrames;
  };
}
