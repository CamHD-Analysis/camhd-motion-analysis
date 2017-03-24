#include <string>
#include <fstream>
#include <sstream>

#include <curlpp/cURLpp.hpp>

#include "camhd_client.h"
#include "interval.h"

#include <opencv2/highgui.hpp>


#include "common_init.h"
#include "movie_workers.h"

using namespace std;
//using namespace cv;

using namespace CamHDMotionTracking;

const fs::path host( "https://camhd-app-dev.appspot.com");
const fs::path url( "/v1/org/oceanobservatories/rawdata/files/RS03ASHS/PN03B/06-CAMHDA301/2016/09/01/CAMHDA301-20160901T000000Z.mov" );



int main( int argc, char ** argv )
{
	auto cleanup = commonInit(argv[0]);



	fs::path videoUrl(host);
	videoUrl /= url;
// 	video_url += host + url;

	auto movie( CamHDClient::getMovie( videoUrl ) );

	// TODO.  Check for failure
	LOG(INFO) << "File has " << movie.numFrames() << " frames";

	Intervals<int> timeline;

	// Force some early bisection
	auto middle = movie.numFrames()/2;
	timeline.add( 1, middle );
	timeline.add( middle, movie.numFrames() );


 const int maxDepth = 10;
	timeline.bisect( CamHDMotionTracking::FrameMean(movie), 10 );

	cout << "-- Results --" << endl << timeline << endl;

	// timeline.add( 0, movie.numFrames );
	//
	// // Start with basic survey
	// const int step = 30;
	//
	// for( auto i = 0; i < movie.numFrames; i += step ) {
	// 	cv::Mat frame( CamHDClient::getFrame( movie, i+1 ));
	//
	// 	// Decimate
	// }





	exit(0);
}
