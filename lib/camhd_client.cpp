
#include <iostream>
#include <vector>

#include <g3log/g3log.hpp>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>

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
  auto result( HTTPRequest::Get( url.string() ) );

  // Check status
  if( result.httpStatus == 200 ) {
   CamHDMovie mov( url.string(), result.body );
   return mov;
  }

  return CamHDMovie();
}

cv::Mat CamHDClient::getFrame( const CamHDMovie &mov, int frame )
{
  // As a special case, if frame > mov.NumFrames(), return a black image
  // (for now, query frame-1 for size and type)
    if( frame > mov.numFrames() || frame <= 0 ) {
      cv::Mat mat( CamHDClient::getFrame( mov, 1 ));
      return cv::Mat::zeros(mat.size(), mat.type());
    }

    fs::path url( CamHDClient::makeFrameURL(mov, frame));

    auto result( HTTPRequest::Get( url.string() ) );

    // Check status
    if( result.httpStatus != 200 ) {
      return cv::Mat();
    }

    cv::Mat in( result.body.size(), 1, CV_8UC1, (void *)result.body.data());

    //cout << "In: " << in.cols << " x " << in.rows << endl;

    cv::Mat out( cv::imdecode(in, cv::IMREAD_UNCHANGED ) );

    //cout << "Out: " << out.cols << " x " << out.rows << endl;


  return out;
}


fs::path CamHDClient::makeFrameURL( const CamHDMovie &mov, int frame )
{
  // Assemble an URL
  fs::path frameUrl( mov.cacheUrl() );
  frameUrl /= "frame";

  std::stringstream fNumStr;
  fNumStr << frame;
  frameUrl /= fNumStr.str();

  //LOG(INFO) << "Getting frame: " << frameUrl.string();


  return frameUrl;
}




}
