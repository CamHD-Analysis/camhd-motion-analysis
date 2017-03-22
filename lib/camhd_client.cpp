
#include <g3log/g3log.hpp>

#include "camhd_client.h"

#include "http_request.h"


namespace CamHDMotionTracking {

using namespace std;

// CamHDClient::CamHDClient(  )
//   // : _baseURL( baseURL )
// {
//   // Query baseURL, potentially rewrite URL based on APIs available?
// }

CamHDMovie CamHDClient::getMovie( const fs::path &url )
{

  // Synchronous
  HTTPRequest request( url.string() );
  request.perform();
  HTTPResult result( request.result() );

  // Check status
  if( result.httpStatus == 200 ) {
    return CamHDMovie( url.string(), result.body );
  }

  return CamHDMovie();
}

cv::Mat CamHDClient::getFrame( const CamHDMovie &mov, int frame )
{
    fs::path url( CamHDClient::makeFrameURL(mov, frame));

    HTTPRequest request( url.string() );
    HTTPResult result( request.result() );

    // Check status
    if( result.httpStatus != 200 ) {
      return cv::Mat();
    }

  return cv::Mat();
}


fs::path CamHDClient::makeFrameURL( const CamHDMovie &mov, int frame )
{
  // Assemble an URL
  fs::path frameUrl( mov.url() );
  frameUrl /= "frame";

  std::stringstream fNumStr;
  fNumStr << frame;
  frameUrl /= fNumStr.str();

  //LOG(INFO) << "Getting frame: " << frameUrl.string();

  return frameUrl;
}




}
