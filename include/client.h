#pragma once

#include <string>


namespace CamHDMotionTracking {

using std::string;

class CamHDClient {
public:

  CamHDClient( const string &baseURL );

  void get( const string &path );

protected:

  string _baseURL;

};

}
