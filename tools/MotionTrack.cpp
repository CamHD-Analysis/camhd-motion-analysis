#include <string>
#include <fstream>
#include <sstream>
//#include <curl/curl.h>

#include <curlpp/cURLpp.hpp>

#include "camhd_client.h"
#include "interval.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

//#include "MotionTrack/MotionTracking.h"

#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>

//#include "kbhit.h"

using namespace std;
//using namespace cv;

using namespace CamHDMotionTracking;

const fs::path host( "https://camhd-app-dev.appspot.com");
const fs::path url( "/v1/org/oceanobservatories/rawdata/files/RS03ASHS/PN03B/06-CAMHDA301/2016/09/01/CAMHDA301-20160901T000000Z.mov" );


namespace CamHDMotionTracking {

struct Frame {
	Frame( int fn, const cv::Mat &m )
		: frameNum(fn), img(m)
	{;}

	int frameNum;
	cv::Mat img;
};

bool operator<( const Frame &a, const Frame &b )
{ return a.frameNum < b.frameNum; }

ostream& operator<<(ostream& os, const Frame& a)
{
		os << "#" << a.frameNum;
		return os;
}

}

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

	Intervals<Frame> timeline;

	// Get the bookends
	cv::Mat zero( CamHDClient::getFrame( movie, 1 ) );
	timeline.add( Frame( 0, zero ),
 								Frame( movie.numFrames(), cv::Mat::zeros( zero.size(), zero.type() )));


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
