#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <curl/curl.h>

#include "json.hpp"
// for convenience
using json = nlohmann::json;

//#include "fmemopen.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

//#include "MotionTrack/MotionTracking.h"

#include "g3log/g3log.hpp"
#include <g3log/logworker.hpp>

//#include "kbhit.h"

using namespace std;
//using namespace cv;

//using namespace CamHD_MotionTracking;

const string host = "camhd-app-dev.appspot.com";
const string url = "/v1/org/oceanobservatories/rawdata/files/RS03ASHS/PN03B/06-CAMHDA301/2016/10/01/";

int main( int argc, char ** argv )
{
	auto worker = g3::LogWorker::createLogWorker();
	g3::initializeLogging(worker.get());

	//CamHD_MotionTracking::Config config( argc, argv );

	//MotionTracking mt( config );
	//mt();

	CURL *conn = curl_easy_init();
	curl_easy_setopt( conn, CURLOPT_FOLLOWLOCATION, 1 );

	string video_url("http://");
	video_url += host + url;

	LOG(INFO) << video_url;
	curl_easy_setopt( conn, CURLOPT_URL, video_url.c_str() );

	// Store JSON to buffer
	FILE *json_buf = fopen("/tmp/json", "w" ); //fmemopen( buf, 2048, "w");

	//curl_easy_setopt( conn, CURLOPT_WRITEFUNCTION, NULL );
	curl_easy_setopt( conn, CURLOPT_WRITEDATA, json_buf );
	//curl_easy_setopt( conn, CURLOPT_MAXFILESIZE, 2047 );


	CURLcode ret = curl_easy_perform( conn );

	if( ret != CURLE_OK ) {
		LOG(FATAL) << "CURL error, returned code: " << ret;
	}

	// Ugliness
	fclose( json_buf );
	auto json_stream = ifstream( "/tmp/json" );

	LOG(INFO) << "CURL return value: " << ret;

	json j3;
	json_stream >> j3;

	std::vector< std::string > files;

	if (j3.find("Files") != j3.end()) {
		auto jFiles = j3["Files"].get< vector<string> >();
		std::copy_if( jFiles.begin(), jFiles.end(), std::back_inserter(files),
	 				[]( const std::string &filename ) { return filename.find(".mov") != string::npos;  }  );
	}

	LOG(INFO) << "Directory has " << files.size() << " files.";

	for( auto file : files ) {
		const int frame = 12500;

	stringstream frame_url;
	frame_url << video_url << file << "/frame/" << (frame == 0 ? 1 : frame);

	LOG(INFO) << frame_url.str();

	curl_easy_setopt( conn, CURLOPT_URL, frame_url.str().c_str() );

	char frame_file[255];
	const string destDir("/auto/canine/aaron/workspace/camhd_analysis/camhd_motion_tracking/sequence");
	sprintf(frame_file, "%s/%s_%d.png", destDir.c_str(), file.c_str(), frame );

	LOG(INFO) << "Saving image to " << frame_file;


	FILE *img_buf = fopen(frame_file, "w" ); //fmemopen( buf, 2048, "w");
	curl_easy_setopt( conn, CURLOPT_WRITEDATA, img_buf );
	ret = curl_easy_perform( conn );

	if( ret != CURLE_OK ) {
		LOG(FATAL) << "CURL error, returned code: " << ret;
	}
	fclose(img_buf);

	cout << "Wrote to: " << frame_file << endl;

}


	curl_easy_cleanup(conn);

	exit(0);
}
