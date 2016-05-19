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

// See: http://cc.byexamples.com/2007/04/08/non-blocking-user-input-in-loop-without-ncurses/
int kbhit()
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

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

		if( !_flowVideoPath.empty() > 0 ) {
			cout << "Saving flow video to " << _flowVideoPath.string() << endl;
			cv::Size sz( frameSize() );
			_flowVideo.open( _flowVideoPath.string(), CV_FOURCC('H','2','6','4'), fps(),
										Size( sz.width*2, sz.height ), true  );

			if( !_flowVideo.isOpened() ) {
				cerr << "Unable to open output video " << _flowVideoPath.string() << ", aborting" << endl;
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

		int frameNum = _capture.get( CV_CAP_PROP_POS_FRAMES );

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

				Mat magInt, angInt;
				mag.convertTo( magInt, CV_8UC1, 255.0/magMax );
				angle.convertTo( angInt, CV_8UC1, 255.0/2*M_PI );

				cvtColor( magInt, magRoi, CV_GRAY2BGR, 3 );
				cvtColor( angInt, angleRoi, CV_GRAY2BGR, 3 );

				if( _doDisplay) {
					imshow("Magnitude", magRoi );
					imshow("Angle", angleRoi );
				}

				if( _flowVideo.isOpened() )
					_flowVideo << composite;
			}

			if( _doDisplay) {
				imshow("MotionTracking", current );

				char ch = waitKey( _waitKey );
				if( ch == 'q' || ch == 'Q') return true;
			} else {
				if( kbhit() ) {
					char ch = fgetc(stdin);
					if( ch == 'q' || ch == 'Q') return true;
				}
			}

			prevGray = curGray;

		if( frameNum % 100 == 0 )
			cout << frameNum << endl;

			if( _stride > 1 ) {
				for( auto i = 0; i < (_stride-1); ++i ) {
					if( !_capture.grab() ) return true;
				}
			}

			// Returns the number of the _next_ frame to be read
		 frameNum = _capture.get( CV_CAP_PROP_POS_FRAMES );

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
		TCLAP::SwitchArg doDisplayArg("x","display","Print name backwards", cmd, false);
		TCLAP::ValueArg< int> waitKeyArg("","wait-key","Number of frames",false,-1,"Number of frames", cmd);

		TCLAP::ValueArg<string> flowVideoOutput("","video-out","",false,"","", cmd);

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
