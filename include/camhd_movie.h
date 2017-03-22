#pragma once

#include <string>


namespace CamHDMotionTracking {
  using std::string;

  class CamHDMovie {
  public:
    CamHDMovie( const string &path, const string &json );

    float duration() const { return _duration; }
    int   numFrames() const { return _numFrames; }

  protected:
      string _path;
      float _duration;
      int _numFrames;
  };
}
