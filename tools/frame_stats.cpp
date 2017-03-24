#include <string>
#include <fstream>
#include <sstream>

#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>

#include <curlpp/cURLpp.hpp>

#include <tclap/CmdLine.h>

#include "camhd_client.h"
#include "interval.h"

#include "movie_workers.h"

#include "json.hpp"
using json = nlohmann::json;

using namespace std;
//using namespace cv;

using namespace CamHDMotionTracking;

const fs::path DefaultCacheURL( "https://camhd-app-dev.appspot.com/v1/org/oceanobservatories/rawdata/files");
//const fs::path DefaultPath( "/RS03ASHS/PN03B/06-CAMHDA301/2016/09/01/CAMHDA301-20160901T000000Z.mov" );



int main( int argc, char ** argv )
{
	auto worker = g3::LogWorker::createLogWorker();
  auto handle= worker->addDefaultLogger(argv[0],".");
  g3::initializeLogging(worker.get());

  // RAAI initializer for curlpp
curlpp::Cleanup cleanup;

	fs::path cacheURL;
	std::string path;
	// Set a default for testing

	fs::path jsonOut;
	int stopAt = -1;
	int stride = 5000;

	try {
		TCLAP::CmdLine cmd("Command description message", ' ', "0.0");

		TCLAP::UnlabeledValueArg<std::string> pathsArg("path","Path",true,"","Path",cmd);
		TCLAP::ValueArg<std::string> jsonOutArg("o", "out", "File for JSON output (leave blank for stdout)", false,jsonOut.string(), "filename", cmd );
		TCLAP::ValueArg<std::string> hostArg("","host","URL to host",false,DefaultCacheURL.string(),"url",cmd);
		TCLAP::ValueArg<int> stopAtArg("","stop-at","",false,stopAt,"frame number",cmd);
		TCLAP::ValueArg<int> strideArg("","stride","",false,stride,"frame number",cmd);



		// Parse the argv array.
		cmd.parse( argc, argv );

		cacheURL = hostArg.getValue();

		stopAt = stopAtArg.getValue();
		stride = strideArg.getValue();

		jsonOut = jsonOutArg.getValue();

		path = pathsArg.getValue();

	} catch (TCLAP::ArgException &e)  {
		LOG(FATAL) << "error: " << e.error() << " for arg " << e.argId();
	}

	// Measure time of execution
	std::chrono::time_point<std::chrono::system_clock> start( std::chrono::system_clock::now() );


	fs::path movURL( cacheURL );
	movURL /= path;

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
		json j( stats(frame) );
		if( !j.empty() ) jsonStats.push_back( j );
	}


std::chrono::duration<double> elapsedSeconds = std::chrono::system_clock::now()-start;

	json mov;
	mov["elapsedSystemTime"] = elapsedSeconds.count();
	mov["movie"] = movie;
	mov["stats"] = jsonStats;


	if( jsonOut.string().size() > 0 ) {
		ofstream f( jsonOut.string() );
		f << mov.dump(4) << endl;
	} else {
		cout << mov.dump(4) << endl;
	}

	exit(0);
}
