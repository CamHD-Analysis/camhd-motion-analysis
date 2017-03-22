

#include "camhd_client.h"

#include "http_request.h"


namespace CamHDMotionTracking {

using namespace std;

CamHDClient::CamHDClient(  )
  // : _baseURL( baseURL )
{
  // Query baseURL, potentially rewrite URL based on APIs available?
}

CamHDMovie CamHDClient::getMovie( const fs::path &url )
{

  // Synchronous
  HTTPRequest request( url.string() );

  HTTPResult result( request.result() );

  // Check status
  if( result.httpStatus == 200 ) {
    return CamHDMovie( url.string(), result.body );
  }

}

cv::Mat CamHDClient::getFrame( const CamHDMovie &mov, int frame )
{

}




}
