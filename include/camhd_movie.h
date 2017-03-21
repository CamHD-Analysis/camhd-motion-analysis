#pragma once

#include <string>


namespace CamHDMotionTracking {
  using std::string;

  class CamHDMovie {
  public:
    CamHDMovie( const string &path, const string &json );

  protected:
      string _path;
  };
}
