#include <string>
#include <fstream>
#include <sstream>

#include <curlpp/cURLpp.hpp>

#include <tclap/CmdLine.h>

#include "camhd_client.h"
#include "interval.h"

#include <opencv2/highgui.hpp>

#include "common_init.h"
#include "movie_workers.h"

#include "json.hpp"
using json = nlohmann::json;

using namespace std;
//using namespace cv;

using namespace CamHDMotionTracking;

const fs::path DefaultCacheURL( "https://camhd-app-dev.appspot.com/v1/org/oceanobservatories/rawdata/files");
const fs::path DefaultPath( "/RS03ASHS/PN03B/06-CAMHDA301/2016/09/01/CAMHDA301-20160901T000000Z.mov" );



int main( int argc, char ** argv )
{

	auto cleanup = commonInit(argv[0]);

	fs::path cacheURL;
	std::vector<fs::path> moviePaths;
	fs::path jsonOut;
	int stopAt = -1;
	int stride = 5000;

	try {
		TCLAP::CmdLine cmd("Command description message", ' ', "0.0");

		TCLAP::ValueArg<std::string> jsonOutArg("o", "output", "File for JSON output (leave blank for stdout)", false,jsonOut.string(), "filename", cmd );
		TCLAP::ValueArg<std::string> hostArg("","host","URL to host",false,DefaultCacheURL.string(),"url",cmd);
		TCLAP::ValueArg<int> stopAtArg("","stop-at","",false,stopAt,"frame number",cmd);
		TCLAP::ValueArg<int> strideArg("","stride","",false,stride,"frame number",cmd);



		// Parse the argv array.
		cmd.parse( argc, argv );

		cacheURL = hostArg.getValue();

		stopAt = stopAtArg.getValue();
		stride = strideArg.getValue();

	} catch (TCLAP::ArgException &e)  {
		LOG(FATAL) << "error: " << e.error() << " for arg " << e.argId();
	}


	// for( auto thisPath : moviePaths ) {

	auto thisPath = DefaultPath;

	fs::path movURL( cacheURL );
	movURL /= thisPath;

	auto movie( CamHDClient::getMovie( movURL ) );



	// TODO.  Check for failure
	LOG(INFO) << "File has " << movie.numFrames() << " frames";


	if( stopAt < 0 )
		stopAt = movie.numFrames();
	else
		stopAt = std::min( movie.numFrames(), stopAt );

	FrameStatistics stats(movie);
	json jsonStats;

	for( auto i = 0; i < stopAt; i += stride ) {
		auto frame = (i==0 ? 1 : i);
		LOG(INFO) << "Processing frame " << frame;
		jsonStats.push_back( stats(frame) );
	}


	json top;
	top["movie"] = movie;

	top["stats"] = jsonStats;


	if( jsonOut.size() > 0 ) {
		ofstream f( jsonOut.string() );
		f << top.dump(4) << endl;
	} else {
		cout << top.dump(4) << endl;
	}

	exit(0);
}
