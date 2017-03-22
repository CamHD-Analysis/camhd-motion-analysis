#include <string>
#include <fstream>
#include <sstream>
//#include <curl/curl.h>

#include <curlpp/cURLpp.hpp>

#include "camhd_client.h"

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

int main( int argc, char ** argv )
{
	auto worker = g3::LogWorker::createLogWorker();
	g3::initializeLogging(worker.get());

	// Initialize curlpp
	curlpp::Cleanup myCleanup;

	//CamHD_MotionTracking::Config config( argc, argv );

	//MotionTracking mt( config );
	//mt();



// 	CURL *conn = curl_easy_init();
// 	curl_easy_setopt( conn, CURLOPT_FOLLOWLOCATION, 1 );
//
	fs::path videoUrl(host);
	videoUrl /= url;
// 	video_url += host + url;

	auto movieInfo( CamHDClient::getMovie( videoUrl ) );

	// TODO.  Check for failure

	LOG(INFO) << "File has " << movieInfo.numFrames() << " frames";


// for( auto frame = 0; frame < maxFrames; frame += 100 ) {
//
//
// 	stringstream frame_url;
// 	frame_url << video_url << "/frame/" << (frame == 0 ? 1 : frame);
//
// 	LOG(INFO) << frame_url.str();
//
// 	curl_easy_setopt( conn, CURLOPT_URL, frame_url.str().c_str() );
//
// 	char frame_file[255];
// 	sprintf(frame_file, "../../images/image_%08d.png", frame );
//
//
// 	FILE *img_buf = fopen(frame_file, "w" ); //fmemopen( buf, 2048, "w");
// 	curl_easy_setopt( conn, CURLOPT_WRITEDATA, img_buf );
// 	ret = curl_easy_perform( conn );
//
// 	if( ret != CURLE_OK ) {
// 		LOG(FATAL) << "CURL error, returned code: " << ret;
// 	}
// 	fclose(img_buf);
//
// 	cout << "Wrote to: " << frame_file << endl;
//
// }
//
//
// 	curl_easy_cleanup(conn);

	exit(0);
}
