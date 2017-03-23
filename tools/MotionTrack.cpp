#include <string>
#include <fstream>
#include <sstream>
//#include <curl/curl.h>

#include <curlpp/cURLpp.hpp>

#include "camhd_client.h"
#include "interval.h"

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

//#include "MotionTrack/MotionTracking.h"

#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>

//#include "kbhit.h"

using namespace std;
//using namespace cv;

using namespace CamHDMotionTracking;

const fs::path host( "https://camhd-app-dev.appspot.com");
const fs::path url( "/v1/org/oceanobservatories/rawdata/files/RS03ASHS/PN03B/06-CAMHDA301/2016/09/01/CAMHDA301-20160901T000000Z.mov" );


struct FrameMean {
	FrameMean( const CamHDMovie &mov )
		: _movie(mov)
		{;}

		bool operator()( int a, int b )
			{ auto meanA = mean( a );
			  auto meanB = mean( b );

				return meanA - meanB; }

		float mean( int frameNum ) {
			cv::Mat frame( CamHDClient::getFrame( _movie, frameNum ));
			cv::Mat reduced;
			cv::resize( frame, reduced, cv::Size(0,0), 0.25, 0.25 );

			auto m = cv::mean( reduced );
			return (m[0]+m[1]+m[2] )/ 3.0;
		}

		CamHDMovie _movie;
};


int main( int argc, char ** argv )
{
	auto worker = g3::LogWorker::createLogWorker();
	auto handle= worker->addDefaultLogger(argv[0],".");
	g3::initializeLogging(worker.get());

	// RAAI initializer for curlpp
	curlpp::Cleanup myCleanup;


	fs::path videoUrl(host);
	videoUrl /= url;
// 	video_url += host + url;

	auto movie( CamHDClient::getMovie( videoUrl ) );

	// TODO.  Check for failure
	LOG(INFO) << "File has " << movie.numFrames() << " frames";

	Intervals<int> timeline;

	// Get the bookends
	timeline.add( 0, movie.numFrames()  );

	timeline.bisect<float>( FrameMean(movie), 1 );

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
