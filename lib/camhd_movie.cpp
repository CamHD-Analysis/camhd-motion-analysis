
#include "camhd_movie.h"
#include "camhd_client.h"

#include "json.hpp"
// for convenience
using json = nlohmann::json;

namespace CamHDMotionTracking {

  CamHDMovie::CamHDMovie( void )
    : _url(), _originalUrl(), _numFrames(0), _duration(0.0)
    {
    }

    CamHDMovie::CamHDMovie( const string &url, const string &json )
      : _url( url ), _originalUrl(), _numFrames(0), _duration(0.0)
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

      if (j3.find("URL") != j3.end()) _originalUrl = j3["URL"];

      }

      cv::Mat CamHDMovie::getFrame( int frame ) {
        if( frame < 1 || frame > numFrames() ) return cv::Mat();

        return CamHDClient::getFrame( *this, frame );
      }

}
