#include <string>
#include <fstream>
#include <sstream>

#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>

#include <curlpp/cURLpp.hpp>

#include <tclap/CmdLine.h>

#include "camhd_client.h"
#include "interval.h"
#include "regions.h"
#include "movie_workers/optical_flow.h"

#include "stitcher.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "json.hpp"
using json = nlohmann::json;

#include "G3LogSinks.h"

using namespace std;
//using namespace cv;

using namespace CamHDMotionTracking;

const fs::path DefaultCacheURL( "https://camhd-app-dev.appspot.com/v1/org/oceanobservatories/rawdata/files");

class StitchConfig {
public:
	StitchConfig()
	: skipZoom(true)
	{}

		bool parseArgs( int argc, char **argv )
		{
			try {
				TCLAP::CmdLine cmd("Command description message", ' ', "0.0");

			//	TCLAP::UnlabeledValueArg<std::string> jsonArg("json","JSON file",true,"","Path",cmd);

				//TCLAP::ValueArg<std::string> jsonInArg("j", "json", "File for JSON output (leave blank for stdout)", false,jsonOut.string(), "filename", cmd );
				TCLAP::ValueArg<std::string> hostArg("","host","URL to host",false,DefaultCacheURL.string(),"url",cmd);

				TCLAP::ValueArg<int> startAtArg("","start-at","",false,startAt,"frame number",cmd);
				TCLAP::ValueArg<int> stopAtArg("","stop-at","",false,stopAt,"frame number",cmd);

				TCLAP::ValueArg<std::string> regionArg("r", "regions", "Region JSON file", true, "", "filename", cmd );
				//TCLAP::ValueArg<int> strideArg("","stride","Number of frames for stride",false,stride,"num of frames",cmd);

				TCLAP::SwitchArg gpuArg("g","gpu","Use GPU",cmd,false);
				TCLAP::SwitchArg displayArg("x","display","Show results in window", cmd, false);

				// Parse the argv array.
				cmd.parse( argc, argv );

				// Args back to
				cacheURL = hostArg.getValue();

				regionFile = regionArg.getValue();

				startAt = startAtArg.getValue();
				startAtSet = startAtArg.isSet();

				stopAt = stopAtArg.getValue();
				stopAtSet = stopAtArg.isSet();

				useGpu = gpuArg.getValue();
				doDisplay = displayArg.getValue();

				//jsonIn = jsonArg.getValue();

			} catch (TCLAP::ArgException &e)  {
				LOG(FATAL) << "error: " << e.error() << " for arg " << e.argId();
			}

			return true;
		}


		fs::path cacheURL;
		//fs::path jsonIn;

		fs::path regionFile;

		int stopAt = -1;
		int startAt = -1;

		bool startAtSet, stopAtSet;
		bool skipZoom;

		bool useGpu, doDisplay;

		//int stride = 5000;

};

int main( int argc, char ** argv )
{
	auto worker = g3::LogWorker::createLogWorker();
  auto handle = worker->addSink(std::unique_ptr<ColorStderrSink>( new ColorStderrSink ),
                                       &ColorStderrSink::ReceiveLogMessage);

  g3::initializeLogging(worker.get());

  // RAAI initializer for curlpp
	curlpp::Cleanup cleanup;

	StitchConfig config;
	if( !config.parseArgs( argc, argv ) ) {
		LOG(WARNING) << "Error while parsing args";
		exit(-1);
	}

	// Measure time of execution
	std::chrono::time_point<std::chrono::system_clock> start( std::chrono::system_clock::now() );

	// Read

	std::ifstream in( config.regionFile.string() );
	json j;
	in >> j;

	CamHDMovie movie( j["movie"].get<CamHDMovie>() );

	Regions regions;
	j >> regions;

	const int baseFrame = 0;

	Graph graph;

	// Heuristic.  Search for first static after a zoom out.

	int i = 1;
	for( ; i < regions.size(); ++i ) {
		if( regions[i-1].is( ZoomOut ) && regions[i].is( Static ) ) break;
	}

int origin = i;
LOG(INFO) << "Starting with section " << origin << " from " << regions[origin].start << " to " << regions[origin].end;


bool zoomedIn = false;
Regions interval;
int destination = -1;
	for( int j = i+1; j < regions.size(); ++j ) {
		// Look ahead for next Static

			if( regions[j].is(ZoomIn) ) {
				zoomedIn = true;
			} else if ( regions[j].is(ZoomOut) ) {
				zoomedIn = false;
			} else if ( !zoomedIn ) {
				if( regions[j].is( Static ) ) {
					destination = j;
					break;
				} else {
					interval.push_back( regions[j] );
				}
			}
	}

	if( destination < 0 ) {
		LOG(WARNING) << "Unable to find destination";
		exit(-1);
	}

LOG(INFO) << "Destination is section " << destination << " from " << regions[destination].start << " to " << regions[destination].end;
LOG(INFO) << "Interval includes " << interval.size() << " regions:";
for( auto r : interval) {
	LOG(INFO) << "     " << r.start << " to " << r.end << " : " << r.typeStr();
}

int refFrame = regions[origin].mean();

LOG(INFO) << "Must bridge from " << regions[origin].end << " to " << regions[destination].start;

cv::Mat originFrame(movie.getFrame( regions[origin].end )),
        destFrame( movie.getFrame( regions[destination].start));

	const int stride = 10;

	Similarity cumulativeSim(1.0);

	for( int frameNum = regions[origin].end; frameNum <regions[destination].start; frameNum += stride ) {
		OpticalFlow flow( movie );

		LOG(INFO) << "Stepping from " << frameNum << " to " << frameNum+stride;
		auto sim = flow.estimateSimilarity( frameNum, frameNum+stride );

		// Heuristic checks here

		cumulativeSim = sim * cumulativeSim;

		LOG(INFO) << "Cumulative similarity: " << cumulativeSim;


		// Align origin to current using cumulativeSim

	}

	// vector< int > frames;
	//
	// transform( regions.begin(), regions.end(), std::back_inserter(frames),
	// 					[]( const Regions::Region &region ) {
	// 						return region.mean(); } );
	//
	// 	// Go sequentially for now
	// 	int lastGood = baseFrame;
	// 	for( auto i = lastGood + 1; i < regions.size(); ++i ) {

	//
	// 		const int f1 = frames[ lastGood ];
	// 		const int f2 = frames[ i ];
	//
	// 		LOG(INFO) << "Comparing " << f1 << " to " << f2;
	// 		auto sim = flow.estimateSimilarity( f1, f2 );
	//
	// 		if( config.doDisplay) {
	// 			imshow("lastGood", movie.getFrame( f1 ) );
	// 			imshow("f2", movie.getFrame( f2 ) );
	//
	// 			waitKey(0);
	// 		}
	//
	// 		// Heuristic tests on sim
	// 		//LOG(INFO) << sim;
	//
	// 		const float scaleThreshold = 0.02;
	//
	// 		if( sim.scale > (1+scaleThreshold) || sim.scale < (1-scaleThreshold) ) {
	// 			LOG(INFO) << "Scale change between " << f1 << " and " << f2 << " == " << sim.scale << ", continuing";
	// 			continue;
	// 		}
	//
	//
	// 		// Success, add edge
	// 		LOG(INFO) << "Adding edge between " << f1 << " and " << f2;
	// 		graph.addEdge( lastGood, i, sim );
	//
	// 		lastGood = i;
	// 	}
	//
	//
	//





	exit(0);

}

// 	CamHDMovie movie( j["movie"].get<CamHDMovie>() );
//
// 	LOG(INFO) << "Loading " << movie.originalUrl() << " from " << movie.cacheUrl();
//
// 	//LOG(INFO) << j["stats"];
//
// 	cv::Mat composite(0,0,CV_8UC3);
// 	cv::Point ul,lr;
// 	cv::Point origin(0,0);
//
// 	// Need to sort stats
// 	json jstats = j["stats"];
// 	std::sort( jstats.begin(), jstats.end(),
// 							[]( const json &a, const json &b ){
// 								return a["frameNum"].get<int>() < b["frameNum"].get<int>();
// 							});
//
// 	for( auto &a : jstats ) {
// 		int frameNum = a["frameNum"];
//
// 		LOG(INFO) << "Processing frame " << frameNum;
//
// 		// TODO:  Validate frameNums
//
// 		if( config.startAtSet && frameNum < config.startAt ) continue;
// 		if( config.stopAtSet && frameNum > config.stopAt ) break;
//
// 		json jsim = a["similarity"];
//
// 		if( jsim.empty() ) {
// 			LOG(INFO) << "Similarity is empty for frame " << frameNum;
// 			continue;
// 		}
//
// 		const bool isValid = jsim["valid"].get<bool>();
// 		LOG(INFO) << "Processing " << frameNum << " : " << (isValid ? "valid" : "invalid");
//
// 		if( !isValid ) {
// 			LOG(INFO) << "Similaritry not valid for frame: " << frameNum;
// 			continue;
// 		}
//
// 		cv::Mat frame( movie.getFrame(frameNum ));
//
// 		if( frame.empty() ) {
// 			LOG(WARNING) << "Got empty frame for " << frameNum;
// 			continue;
// 		}
//
// 		auto coeffs( jsim["similarity"] );
//
// 		const float s( coeffs[0].get<float>() ),
// 								theta( coeffs[1].get<float>() );
// 		const cv::Vec2f tx( coeffs[2].get<float>(), coeffs[3].get<float>() );
// 		const cv::Point center(frame.size().width / 2.0, frame.size().height / 2.0 );
//
// 		LOG(INFO) << coeffs;
//
// 		// Project corners of image into composite
// 		cv::Matx33d cam( 1, 0, -center.x, 0, 1, -center.y, 0, 0, 1 );
// 		cv::Matx33d sim( s*cos(theta), s*sin(theta), tx[0],
// 		                    -s*sin(theta), s*cos(theta), tx[1],
// 											 0, 0, 1 );
//
// 	 LOG(INFO) << "Cam: " << cam;
// 	 LOG(INFO) << "Sim: " << sim;
//
// 		array<cv::Point,4> corners = { cv::Point(0,0),
// 																		cv::Point(frame.size().width, 0),
// 																		cv::Point(frame.size().width, frame.size().height ),
// 																		cv::Point(0,frame.size().height)};
//
// 		cv::Matx33d warp( cam.inv() * sim.inv() * cam );
//
// 		for( auto &corner : corners ) {
// 			auto proj = warp * cv::Vec3d( corner.x, corner.y, 1 );
// 			cv::Point p( proj[0]/proj[2], proj[1]/proj[2]);
//
// 			ul.x = std::min( ul.x, p.x );
// 			ul.y = std::min( ul.y, p.y );
// 			lr.x = std::max( lr.x, p.x );
// 			lr.y = std::max( lr.y, p.y );
// 		}
//
// 		const int width = lr.x - ul.x;
// 		const int height = lr.y - ul.y;
//
// LOG(INFO) << "ul: " << ul;
// LOG(INFO) << "lr: " << lr;
//
// LOG(INFO) << "width: " << width << " ; height: " << height;
//
//
//  		if( width > composite.size().width || height > composite.size().height ) {
// 			LOG(INFO) << "Need to expand composite to " << width << " x " << height;
// 		}
//
// 		cv::Mat newComposite( cv::Mat::zeros(height, width, composite.type() ) );
//
// 		// Copy old composite into new
// 		if( !composite.empty() ) {
// 			cv::Rect roi( origin.x - ul.x, origin.y - ul.y, composite.size().width, composite.size().height );
// 			cv::Mat oldCRoi( newComposite, roi );
// 			composite.copyTo( oldCRoi );
// 		}
//
// 		//cv::imshow( "newC", newComposite );
//
// 		// Add warp into newC
// 		// Recompute origin
// 		origin = cv::Point( -ul.x, -ul.y );
// 		cv::Matx33d totalWarp = cv::Matx33d(1, 0, origin.x, 0, 1, origin.y, 0, 0, 1) * warp;
//
// 		cv::Mat newWarped( cv::Mat::zeros(newComposite.size(), newComposite.type() ) );
// 		cv::warpPerspective( frame, newWarped, totalWarp, newComposite.size() );
//
// 		// Draw an outline around the
// 	for( int i = 0; i < 4; ++i ) {
// 		int j = (i == 3) ? 0 : i+1;
//
// 		auto c1 = totalWarp * cv::Vec3d( corners[i].x, corners[i].y, 1.0 );
// 		auto c2 = totalWarp * cv::Vec3d( corners[j].x, corners[j].y, 1.0 );
//
// 		cv::Point corner1( c1[0]/c1[2], c1[1]/c1[2] );
// 		cv::Point corner2( c2[0]/c2[2], c2[1]/c2[2] );
//
// 		cv::line( newWarped, corner1, corner2, cv::Scalar(0,0,255), 3 );
//
// 	}
//
// 	cv::addWeighted( newComposite, 0.5, newWarped, 0.5, 0.0, newComposite );
//
// 		composite = newComposite;
//
// 		cv::imshow("composite", composite);
// 		cv::waitKey(0);
//
// 	}
//
//
// 	exit(0);
// }
