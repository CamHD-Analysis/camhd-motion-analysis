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


//namespace CamHDMotionTracking {
//
// struct Frame {
// 	Frame( const CamHDMovie &mov, int fn )
// 		: movie(mov), frameNum(fn)
// 	{;}
//
// 	const cv::Mat getFrame() {
// 		return CamHDClient::getFrame( movie, frameNum );
// 	}
//
// 	const cv::Mat reducedFrame() {
// 		if( reduced.empty() ) {
// 		cv::Mat full( getFrame() );
//
// 		cv::resize( full, reduced, cv::Size(), 0.25, 0.25 );
// 		}
//
// 		return reduced;
// 	}
//
// 	cv::Scalar mean() {
// 		return cv::mean( reduced );
// 	}
//
// 	CamHDMovie movie;
// 	int frameNum;
// 	cv::Mat reduced;
//
// };
//
// bool operator<( const Frame &a, const Frame &b )
// { return a.frameNum < b.frameNum; }
//
// ostream& operator<<(ostream& os, const Frame& a)
// {
// 		os << "#" << a.frameNum;
// 		return os;
// }
//

typedef Interval<int> FrameInterval;

// class FrameInterval : public Interval<int> {
// public:
//
// 	FrameInterval( const Frame &start, const Frame &end )
// 		: Interval<int>(start.frameNum, end.frameNum)
// 		{;}
//
// protected:
//
// };

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

	Intervals< FrameInterval> timeline;

	// Get the bookends
	timeline.add( FrameInterval( 0, movie.numFrames() )  );

	timeline.bisect();

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
