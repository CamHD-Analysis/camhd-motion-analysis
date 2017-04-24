
#include "camhd_movie.h"
#include "camhd_client.h"

#include "json.hpp"
// for convenience
using json = nlohmann::json;

namespace CamHDMotionTracking {

  CamHDMovie::CamHDMovie( void )
    : _cacheUrl(), _originalUrl(), _numFrames(0), _duration(0.0), _initialized(false)
    {
    }

  CamHDMovie::CamHDMovie( const CamHDMovie &other )
    : _cacheUrl( other.cacheUrl() ), _originalUrl( other.originalUrl() ),
      _numFrames( other.numFrames() ), _duration( other.duration() ), _initialized(true)
  {
    ;
  }

    CamHDMovie::CamHDMovie( const string &url, const string &jsonStr )
      : _cacheUrl( url ), _originalUrl(), _numFrames(0), _duration(0.0), _initialized(false)
    {
      if( jsonStr.size() > 0 ) {
        json j = json::parse( jsonStr );
        from_json(j, *this);
      }

      _initialized = true;

      //
      // std::cout << "url: " << url << std::endl;
      // std::cout << "_cacheUrl: " << _cacheUrl << std::endl;
    }

    cv::Mat CamHDMovie::getFrame( int frame ) {
      if( frame < 1 || frame > numFrames() ) return cv::Mat();

      return CamHDClient::getFrame( *this, frame );
    }

}
