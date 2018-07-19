#include <string>
#include <fstream>
#include <sstream>

#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>

#include <curlpp/cURLpp.hpp>

#include <tclap/CmdLine.h>

#include "camhd_client.h"
#include "interval.h"

#include "movie_workers/frame_processor.h"
#include "movie_workers/frame_mean.h"
#include "movie_workers/frame_statistics.h"
#include "movie_workers/approx_derivative.h"

#include "json.hpp"
using json = nlohmann::json;

using namespace std;
//using namespace cv;

using namespace CamHDMotionTracking;

const fs::path DefaultCacheURL( "https://camhd-app-dev.appspot.com/v1/org/oceanobservatories/rawdata/files");

class AnnotateConfig {
public:
	AnnotateConfig()
	{}

		bool parseArgs( int argc, char **argv )
		{
			try {
				TCLAP::CmdLine cmd("Command description message", ' ', "0.0");

				TCLAP::UnlabeledValueArg<std::string> jsonArg("json","JSON file",true,"","Path",cmd);

				//TCLAP::ValueArg<std::string> jsonInArg("j", "json", "File for JSON output (leave blank for stdout)", false,jsonOut.string(), "filename", cmd );
				TCLAP::ValueArg<std::string> hostArg("","host","URL to host",false,DefaultCacheURL.string(),"url",cmd);

				TCLAP::ValueArg<int> startAtArg("","start-at","",false,startAt,"frame number",cmd);
				TCLAP::ValueArg<int> stopAtArg("","stop-at","",false,stopAt,"frame number",cmd);
				//TCLAP::ValueArg<int> strideArg("","stride","Number of frames for stride",false,stride,"num of frames",cmd);

				// Parse the argv array.
				cmd.parse( argc, argv );

				// Args back to
				cacheURL = hostArg.getValue();

				startAt = startAtArg.getValue();
				startAtSet = startAtArg.isSet();

				stopAt = stopAtArg.getValue();
				stopAtSet = stopAtArg.isSet();

				jsonIn = jsonArg.getValue();

			} catch (TCLAP::ArgException &e)  {
				LOG(FATAL) << "error: " << e.error() << " for arg " << e.argId();
			}

			return true;
		}


		fs::path cacheURL;
		fs::path jsonIn;

		int stopAt = -1;
		int startAt = -1;

		bool startAtSet, stopAtSet;

		//int stride = 5000;

};

int main( int argc, char ** argv )
{
	auto worker = g3::LogWorker::createLogWorker();
  auto handle= worker->addDefaultLogger(argv[0],".");
  g3::initializeLogging(worker.get());

  // RAAI initializer for curlpp
	curlpp::Cleanup cleanup;

	AnnotateConfig config;
	if( !config.parseArgs( argc, argv ) ) {
		LOG(WARNING) << "Error while parsing args";
		exit(-1);
	}

	// Measure time of execution
	std::chrono::time_point<std::chrono::system_clock> start( std::chrono::system_clock::now() );

	std::ifstream in( config.jsonIn.string() );
	json j;
	in >> j;

	CamHDMovie movie( j["movie"].get<CamHDMovie>() );

	LOG(INFO) << "Loading " << movie.originalUrl() << " from " << movie.cacheUrl();

	//LOG(INFO) << j["stats"];

	for( auto &a : j["stats"] ) {
		int frameNum = a["frameNum"];
		// TODO:  Validate frameNums

		if( config.startAtSet && frameNum < config.startAt ) continue;
		if( config.stopAtSet && frameNum > config.stopAt ) break;

		json sim = a["similarity"];

		if( sim.empty() ) continue;

		const bool isValid = sim["valid"].get<bool>();
		LOG(INFO) << "Processing " << frameNum << " : " << (isValid ? "valid" : "invalid");

		if( !isValid ) continue;

		cv::Mat frame( movie.getFrame(frameNum ));

		auto coeffs( sim["similarity"] );

		const float s( coeffs[0].get<float>() ),
								theta( coeffs[1].get<float>() );
		const cv::Vec2f tx( coeffs[2].get<float>(), coeffs[3].get<float>() );

		const cv::Point center(frame.size().width / 2.0, frame.size().height / 2.0 );

		LOG(INFO) << coeffs;



		// Draw translation
		const float lineScale = -1.0;   // Negative scaling so it points into direction of motion
		cv::line( frame, center,
										cv::Point(center.x + lineScale*tx[0], center.y + lineScale*tx[1] ),
										cv::Scalar(255,0,0), 5 );

		LOG(INFO) << "Scale: " << s;

		// Draw scaling
		const float zoomScale = 1000;
		const float zoomRadius = std::abs(s-1.0) * zoomScale;
		cv::Scalar zoomColor( ( s > 1.0 ) ? cv::Scalar(0,255,0) : cv::Scalar(0,0,255) );

		cv::circle( frame, center, zoomRadius, zoomColor, 2 );

		imshow( "frame", frame );
		waitKey(30);
	}

	//
	// // TODO.  Check for failure
	// LOG(INFO) << "File has " << movie.numFrames() << " frames";
	//
	// std::vector< std::shared_ptr< FrameProcessor > > processors;
	// //processor = new FrameStatistics stats(movie);
	// processors.emplace_back( new ApproxDerivative(movie) );
	// processors.emplace_back( new FrameStatistics(movie) );
	//
	// json jsonStats;
	//
	// const int startAt = (config.startAtSet ? std::max( 0, config.startAt ) : 0 );
	// const int stopAt = (config.stopAtSet ? std::min( movie.numFrames(), config.stopAt ) : movie.numFrames() );
	//
	// for( auto i = startAt; i < stopAt; i += config.stride ) {
	// 	auto frame = (i==0 ? 1 : i);
	// 	LOG(INFO) << "Processing frame " << frame;
	// 	json j;
	//
  //   j["frameNum"] = i;
	//
	// 	for( auto proc : processors ) {
	// 		j[proc->jsonName()] = proc->process(frame);
	// 	}
	//
	// 	if( !j.empty() ) jsonStats.push_back( j );
	// }
	//
	//
	// std::chrono::duration<double> elapsedSeconds = std::chrono::system_clock::now()-start;
	//
	// json mov;
	// mov["elapsedSystemTime"] = elapsedSeconds.count();
	// mov["movie"] = movie;
	// mov["stats"] = jsonStats;
	//
	//
	// if( config.jsonOutSet ) {
	// 	ofstream f( config.jsonOut.string() );
	// 	f << mov.dump(4) << endl;
	// } else {
	// 	cout << mov.dump(4) << endl;
	// }

	exit(0);
}
