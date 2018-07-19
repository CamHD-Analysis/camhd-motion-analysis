#include <string>
#include <fstream>
#include <sstream>
#include <chrono>

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

#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>

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
			try {
				TCLAP::CmdLine cmd("Command description message", ' ', "0.0");

				TCLAP::UnlabeledValueArg<std::string> pathsArg("mov-path","path",true,"","path",cmd);

				TCLAP::ValueArg<std::string> jsonOutArg("o", "out", "File for JSON output (leave blank for stdout)", false,jsonOut.string(), "filename", cmd );
				TCLAP::ValueArg<std::string> hostArg("","host","URL to host",false,DefaultCacheURL.string(),"url",cmd);

				TCLAP::ValueArg<int> parallelismArg("j","parallelism","Number of threads", false, -1, "num of threads", cmd);

				TCLAP::ValueArg<int> startAtArg("","start-at","",false,startAt,"frame number",cmd);
				TCLAP::ValueArg<int> stopAtArg("","stop-at","",false,stopAt,"frame number",cmd);
				TCLAP::ValueArg<int> strideArg("","stride","Number of frames for stride",false,stride,"num of frames",cmd);

				TCLAP::ValueArg<int> frameArg("","frame","",false,0,"frame number", cmd);


				TCLAP::SwitchArg gpuArg("g","gpu","Use GPU",cmd,false);
				TCLAP::SwitchArg displayArg("x","display","Show results in window", cmd, false);

				// Parse the argv array.
				cmd.parse( argc, argv );

				// Args back to
				cacheURL = hostArg.getValue();

				parallelismSet = parallelismArg.isSet();
				parallelism = parallelismArg.getValue();

				if( frameArg.isSet() ) {
					startAt = frameArg.getValue();
					stopAt = startAt+1;
					stride = 0;
				} else {
					startAt = startAtArg.getValue();
					startAtSet = startAtArg.isSet();

					stopAt = stopAtArg.getValue();
					stopAtSet = stopAtArg.isSet();

					stride = strideArg.getValue();
				}

				jsonOut = jsonOutArg.getValue();
				jsonOutSet = jsonOutArg.isSet();

				useGpu = gpuArg.getValue();
				doDisplay = displayArg.getValue();

				path = pathsArg.getValue();

			} catch (TCLAP::ArgException &e)  {
				LOG(FATAL) << "error: " << e.error() << " for arg " << e.argId();
			}

			return true;
		}


		fs::path cacheURL;
		std::string path;
		// Set a default for testing

		fs::path jsonOut;
		bool jsonOutSet;

		int parallelism, numThreads;
		bool parallelismSet;

		bool useGpu, doDisplay;

		int stopAt = -1;
		int startAt = -1;

		bool startAtSet, stopAtSet;

		int stride = 5000;

};

int main( int argc, char ** argv )
{
	auto worker = g3::LogWorker::createLogWorker();
  //auto handle= worker->addDefaultLogger(argv[0],".");
  g3::initializeLogging(worker.get());

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

#ifdef USE_OPENMP
	if( config.parallelismSet && config.parallelism > 0 ) {
		omp_set_num_threads( config.parallelism );
	}
#endif

	// Measure time of execution
	std::chrono::time_point<std::chrono::system_clock> start( std::chrono::system_clock::now() );

	fs::path movURL( config.cacheURL );
	movURL /= config.path;

	auto movie( CamHDClient::getMovie( movURL ) );

	if( !movie.initialized() ) {
		LOG(FATAL) << "Couldn't get information about movie.";
	}

	// TODO.  Check for failure
	LOG(INFO) << "File has " << movie.numFrames() << " frames";

	std::vector< FrameProcessorFactory::Ptr > factories;
	//processor = new FrameStatistics stats(movie);
#ifdef USE_GPU
	if( config.useGpu ) {
		LOG(INFO) << "Using GPU-based optical flow";
		auto factory = new GPUOpticalFlowFactory();
		factory->doDisplay = config.doDisplay;
		factories.emplace_back( factory );
	} else
#endif
	{
		auto factory = new OpticalFlowFactory();
		factory->doDisplay = config.doDisplay;
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


	const int startAt = (config.startAtSet ? std::max( 0, config.startAt ) : 0 );
	const int stopAt = (config.stopAtSet ? std::min( movie.numFrames(), config.stopAt ) : movie.numFrames() );

	#pragma omp parallel for shared(jsonStats)
	for( auto i = startAt; i < stopAt; i += config.stride ) {

		// This might seem like a funny way to break from a loop, but OpenMP does
		// not allow breaking from within loops
		if( !doStop ) {
			auto frame = (i==0 ? 1 : i);
			LOG(INFO) << "Processing frame " << frame;
			json j = json::object();

	    j["frameNumber"] = i;

			for( auto factory : factories ) {
				Timer processTimer;

				auto proc = (*factory)(movie);
				json data = proc->asJson(frame);


				LOG(INFO) << "Name " << proc->jsonName() << endl;

				j[ proc->jsonName() ] = data;
				j["durationSeconds"] = processTimer.seconds();


			}

			#pragma omp critical
			{
				if( !j.empty() ) jsonStats.push_back( j );
				mov["frameStats"] = jsonStats;

				// Save incremental result
				if( config.jsonOutSet ) {
					ofstream f( config.jsonOut.string() );
					f << mov.dump(4) << endl;
				}
			}
		}
	}

	std::chrono::duration<double> elapsedSeconds = std::chrono::system_clock::now()-start;
	//
	// addJSONContents( mov, "timing", "1.0" );
	// mov["timing"]["elapsed_system_time_s"] = elapsedSeconds.count();

		if( config.jsonOutSet ) {
			ofstream f( config.jsonOut.string() );
			f << mov.dump(4) << endl;
		} else {
			cout << mov.dump(4) << endl;
		}

		LOG(DEBUG) << "Completed in " << elapsedSeconds.count() << " seconds";


	exit(0);
}
