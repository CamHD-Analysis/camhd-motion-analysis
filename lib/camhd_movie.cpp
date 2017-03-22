
#include "camhd_movie.h"

#include "json.hpp"
// for convenience
using json = nlohmann::json;

namespace CamHDMotionTracking {

    CamHDMovie::CamHDMovie( const string &path, const string &json )
      : _path( path ), _url(), _numFrames(0), _duration(0.0)
      {
        metadataFromJson( json );
      }


      void CamHDMovie::metadataFromJson( const string &metadata )
      {
        auto j3 = json::parse( metadata );
        // 	json_stream >> j3;
        //
        // 	int maxFrames = -1;
        //
        	if (j3.find("NumFrames") != j3.end()) _numFrames = j3["NumFrames"];

        if (j3.find("Duration") != j3.end())   _duration = j3["Duration"];

      if (j3.find("URL") != j3.end()) _url = j3["URL"];

      }

}
