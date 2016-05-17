#include <string>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video.hpp>

#include <boost/filesystem.hpp>
#include <tclap/CmdLine.h>

namespace fs = boost::filesystem;

using namespace std;
using namespace cv;

class MotionTracking
{
public:
	MotionTracking( const fs::path &inFile )
		: _inputFilePath( inFile ),
			_flowVideoPath(),
			_capture( inFile.string() ),
			_stride(10), _skip(0),
			_doDisplay( false ),
			_waitKey(-1),
			_opticalFlow( createOptFlow_DualTVL1() )
	{}

	~MotionTracking()
	{
		if( _capture.isOpened() ) _capture.release();
	}

	unsigned int setStride( unsigned int c ) { return _stride=c;}
	unsigned int setSkip( unsigned int c ) { return _skip=c; }
	bool setDoDisplay( bool d ) { return _doDisplay=d; }
	int setWait( int w ) { return _waitKey = w; }
	void setFlowVideoOutput( const fs::path &p ) { _flowVideoPath = p; }

	bool operator()( void )
	{
		if( !_capture.isOpened() ) {
			cerr << "Unable to open video file.  Aborting." << endl;
			return false;
		}

		if( _flowVideoPath.size() > 0 ) {
			cout << "Saving flow video to " << _flowVideoPath.string() << endl;
			_flowVideo.open( _flowVideoPath.string(), int(_capture.get(CV_CAP_PROP_FOURCC)), fps(), frameSize(), true  );

			if( !_flowVideo.isOpened() ) {
				cerr << "Unabled to open output video " << _flowVideoPath.string() << ", aborting";
				return false;
			}
		}


		cv::Size sz( frameSize() );
		cout << "Video frames are " << sz.width << " x " << sz.height << endl;
		cout << "Video is " << frameCount() << " frames long" << endl;
		cout << "    at " << fps() << " fps" << endl;

		if( _waitKey < 0 ) {
			float f = fps();
			if( f <= 0.0 ) f = 29.97;
			_waitKey = 1000 * 1.0/f;
		}

		if( _skip > 0 ) {
			for( auto i = 0; i < _skip; ++i ) {
				if( !_capture.grab() )  {
					cerr << "Reached end of file while skipping." << endl;
					return false;
				}
			}
		}

		Mat current, prevGray;
		while( _capture.read(current) ) {

			Mat curGray, mag, angle;

			cvtColor( current, curGray, CV_BGR2GRAY );

			if( !prevGray.empty() ) {
				Mat flow;
				_opticalFlow->calc( prevGray, curGray, flow );

				std::vector<cv::Mat> components(2);
				split( flow, components );

				cartToPolar( components[0], components[1], mag, angle );
			}

			if( !mag.empty() && (_flowVideo.isOpened() || _doDisplay) ) {
				Mat composite( cv::Size(2*mag.cols, mag.rows), CV_8UC3 );
				Mat magRoi( composite, cv::Rect(0,0, mag.cols, mag.rows ) ),
						angleRoi( composite, cv::Rect( mag.cols, 0, angle.cols, angle.rows ));

				double magMin, magMax = 1;
				minMaxLoc( mag, &magMin, &magMax );

				cvtColor( mag/magMax, magRoi, CV_GRAY2BGR );
				cvtColor( angle / 2*M_PI, angleRoi, CV_GRAY2BGR );

				if( _doDisplay) {
					imshow("Magnitude", magRoi );
					imshow("Angle", angleRoi );
				}

				if( _flowVideo.isOpened() )
					_flowVideo << composite;
			}

			if( _doDisplay) {
				imshow("MotionTracking", current );

				waitKey( _waitKey );
			}

			prevGray = curGray;


			if( _stride > 1 ) {
				for( auto i = 0; i < (_stride-1); ++i ) {
					if( !_capture.grab() ) return true;
				}
			}
		}


		return true;
	}

	cv::Size frameSize( void )
	{
		return cv::Size( _capture.get(CV_CAP_PROP_FRAME_WIDTH),
										_capture.get(CV_CAP_PROP_FRAME_HEIGHT) );
	}

	double frameCount( void )
	{ return _capture.get(CV_CAP_PROP_FRAME_COUNT); }

	double fps( void )
	{ return _capture.get(CV_CAP_PROP_FPS); }



protected:

	fs::path _inputFilePath, _flowVideoPath;
	VideoCapture _capture;
	VideoWriter _flowVideo;
	unsigned int _stride, _skip;
	bool _doDisplay;
	int _waitKey;

Ptr<DenseOpticalFlow> _opticalFlow;

};


int main( int argc, char ** argv )
{

	try {
		TCLAP::CmdLine cmd("Frame-by-frame motion tracking from CamHD files", ' ', "");

		TCLAP::ValueArg<unsigned int> skipArg("","skip","Number of frames",false,0,"Number of frames", cmd);
		TCLAP::ValueArg<unsigned int> strideArg("s","stride","Number of frames",false,10,"Number of frames", cmd);
		TCLAP::SwitchArg doDisplayArg("x","display","Print name backwards", cmd, true);
		TCLAP::ValueArg< int> waitKeyArg("","wait-key","Number of frames",false,-1,"Number of frames", cmd);

		TCLAP::ValueArg<string> flowVideoOutput("","magnitude-video","",false,"","", cmd);

		TCLAP::UnlabeledValueArg<string> filenameArg("input-file", "Input file", true, "", "Input filename", cmd );

		// Parse the argv array.
		cmd.parse( argc, argv );

		fs::path inputFilePath( filenameArg.getValue() );
		if( !fs::exists(inputFilePath) || !fs::is_regular_file( inputFilePath) ) {
			cerr << "error: " << inputFilePath.string() << " doesn't exist or isn't readable!" << endl;
			exit(-1);
		}

		MotionTracking mt( inputFilePath );
		mt.setSkip( skipArg.getValue() );
		mt.setStride( strideArg.getValue() );
		mt.setDoDisplay( doDisplayArg.getValue() );
		mt.setWait( waitKeyArg.getValue() );
		mt.setFlowVideoOutput( flowVideoOutput.getValue() );
		mt();


	} catch (TCLAP::ArgException &e)  // catch any exceptions
	{
		cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
	}



	exit(0);
}
