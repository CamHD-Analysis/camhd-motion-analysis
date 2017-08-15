#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <signal.h>

#ifdef USE_OPENMP
#include <omp.h>
#endif

#include <curlpp/cURLpp.hpp>

#include <tclap/CmdLine.h>

#include "camhd_client.h"
#include "interval.h"
#include "timer.h"

#include "movie_workers/frame_processor.h"
#include "movie_workers/frame_mean.h"
#include "movie_workers/frame_statistics.h"
#include "movie_workers/approx_derivative.h"
#include "movie_workers/optical_flow.h"
#ifdef USE_GPU
#include "movie_workers/gpu_optical_flow.h"
#endif

#include "json.hpp"
using json = nlohmann::json;

// #include <g3log/g3log.hpp>
// #include <g3log/logworker.hpp>

#include <glog/logging.h>

using namespace std;
//using namespace cv;

using namespace CamHDMotionTracking;

const fs::path DefaultCacheURL( "https://camhd-app-dev.appspot.com/v1/org/oceanobservatories/rawdata/files");


bool doStop = false;

void catchSignal(int signo) {
	switch( signo ) {
		case SIGINT:
				doStop = true;
				return;
	}
}



class FrameStatsConfig {
public:
	FrameStatsConfig()
	{}

		bool parseArgs( int argc, char **argv )
		{
			FLAGS_logtostderr = true;
			FLAGS_minloglevel = 0;
			google::InitGoogleLogging(argv[0]);

			try {
				TCLAP::CmdLine cmd("Command description message", ' ', "0.0");

				TCLAP::UnlabeledValueArg<std::string> pathArg("mov-path","path",true,"","path",cmd);

				TCLAP::ValueArg<int> frameArg("","frame","",true,0,"frame number", cmd);

				TCLAP::ValueArg<std::string> lazycacheUrlArg("","lazycache-url","URL to host",false, DefaultCacheURL.string(),"url",cmd);

				TCLAP::SwitchArg gpuArg("g","gpu","Use GPU",cmd,false);

				// Parse the argv array.
				cmd.parse( argc, argv );

				// Args back to
				cacheURL = lazycacheUrlArg.getValue();

				frame = frameArg.getValue();

				useGpu = gpuArg.getValue();

				path = pathArg.getValue();

			} catch (TCLAP::ArgException &e)  {
				LOG(FATAL) << "error: " << e.error() << " for arg " << e.argId();
			}

			return true;
		}


		fs::path cacheURL;
		std::string path;
		// Set a default for testing

		bool useGpu;

		int frame;

};

int main( int argc, char ** argv )
{
  // RAAI initializer for curlpp
	curlpp::Cleanup cleanup;

	if(signal(SIGINT, catchSignal ) == SIG_ERR) {
			LOG(FATAL) << "An error occurred while setting the signal handler.";
			exit(-1);
	}

	FrameStatsConfig config;
	if( !config.parseArgs( argc, argv ) ) {
		LOG(WARNING) << "Error while parsing args";
		exit(-1);
	}

	// Measure time of execution
	Timer mainTimer;

	fs::path movURL( config.cacheURL );
	movURL /= config.path;

	auto movie( CamHDClient::getMovie( movURL ) );

	if( !movie.initialized() ) {
		LOG(FATAL) << "Couldn't get information about movie.";
	}

	// TODO.  Check for failure
	LOG(INFO) << "File has " << movie.numFrames() << " frames";

	std::vector< FrameProcessorFactory::Ptr > factories;


#ifdef USE_GPU
	if( config.useGpu ) {
		LOG(INFO) << "Using GPU-based optical flow";
		auto factory = new GPUOpticalFlowFactory();
		factory->doDisplay = false;
		factories.emplace_back( factory );
	} else
#endif
	{
		auto factory = new OpticalFlowFactory();
		factory->doDisplay = false;
		factories.emplace_back( factory );
	}
	//processors.emplace_back( new FrameStatistics(movie) );

	json mov, jsonStats;

	mov << movie;
	mov["contents"] = json::object();

	//addJSONContents( mov["contents"], "frame_stats", "1.1" );

	mov["contents"]["frameStats"] = json::object();

	for( auto factory : factories ) {
		factory->addJSONContents( mov["contents"]["frameStats"] );
	}

	const int frameNumber = config.frame;

	LOG(INFO) << "Processing frame " << frameNumber;
	json j = json::object();

  j["frameNumber"] = frameNumber;

	for( auto factory : factories ) {
		Timer processTimer;

		auto proc = (*factory)(movie);
		json data = proc->asJson(frameNumber);

		j[ proc->jsonName() ] = data;
		j["durationSeconds"] = processTimer.seconds();


	}

	if( !j.empty() ) jsonStats.push_back( j );
	mov["frameStats"] = jsonStats;

	// addJSONContents( mov, "timing", "1.0" );
	// mov["timing"]["elapsed_system_time_s"] = elapsedSeconds.count();

	// JSON to stdout
	cout << mov.dump(4) << endl;

		LOG(INFO) << "Completed in " << mainTimer.seconds() << " seconds";


	exit(0);
}
