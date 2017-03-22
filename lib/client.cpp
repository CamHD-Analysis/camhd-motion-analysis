

#include "client.h"

#include "http_request.h"


namespace CamHDMotionTracking {

using namespace std;

CamHDClient::CamHDClient( const string &baseURL )
  : _baseURL( baseURL )
{
  // Query baseURL, potentially rewrite URL based on APIs available?
}

CamHDMovie CamHDClient::getMovie( const string &path )
{
  fs::path moviePath( _baseURL );
  moviePath /= fs::path(path);

  // Synchronous
  HTTPRequest request( moviePath.string() );

  HTTPResult result( request.result() );

  // Check status
  if( result.httpStatus == 200 ) {
    return CamHDMovie( path, result.body );
  }

}

cv::Mat CamHDClient::getFrame( const CamHDMovie &mov, int frame )
{

}




}
