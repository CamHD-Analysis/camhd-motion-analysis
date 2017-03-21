

#include "client.h"

#include "http_request.h"


namespace CamHDMotionTracking {

using namespace std;

CamHDClient::CamHDClient( const string &baseURL )
  : _baseURL( baseURL )
{

}

CamHDMovie CamHDClient::getMovie( const string &path )
{
  string moviePath = _baseURL + path;

  // Synchronous
  HTTPRequest request( moviePath );

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
