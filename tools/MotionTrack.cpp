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

class MotionTrackingConfig {
public:

	MotionTrackingConfig( int argc, char **argv )
		: _cmd("Frame-by-frame motion tracking from CamHD files", ' ', ""),
			_skipArg("","skip","Number of frames",false,0,"Number of frames", _cmd),
			_strideArg("s","stride","Number of frames",false,10,"Number of frames", _cmd),
			_doDisplayArg("x","display","Print name backwards", _cmd, true),
			_waitKeyArg("","wait-key","Number of frames",false,-1,"Number of frames", _cmd),
			_scaleArg("","scale","Scale",false,1.0,"Scale",_cmd),
			_videoOutputArg("","video-out","",false,"","", _cmd),
			_videoInputArg("input-file", "Input file", true, "", "Input filename", _cmd )
	{
		parse( argc, argv );
	}

	void parse( int argc, char **argv )
	{
		try {
			_cmd.parse( argc, argv );

			_videoInputPath = _videoInputArg.getValue();
			if( !fs::exists(_videoInputPath) || !fs::is_regular_file( _videoInputPath) ) {
				cerr << "error: " << _videoInputPath.string() << " doesn't exist or isn't readable!" << endl;
				exit(-1);
			}

		} catch (TCLAP::ArgException &e)  // catch any exceptions
		{
			cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
		}

		_waitKey = _waitKeyArg.getValue();
	}

#define TCLAP_ACCESSOR( type, name ) type name( void ) { return _##name##Arg.getValue(); }

	TCLAP_ACCESSOR( unsigned int, skip )
	TCLAP_ACCESSOR( unsigned int, stride )
	TCLAP_ACCESSOR( float, scale )
	TCLAP_ACCESSOR( fs::path, videoOutput )
	TCLAP_ACCESSOR( bool, doDisplay )


	std::string inputFileString( void ) const { return _videoInputPath.string(); }

	bool isVideoOutputSet( void ) const { return _videoOutputArg.isSet(); }

	float updateWaitKey( float fps )
	{
		if( _waitKey < 0 ) {
			if( fps <= 0.0 ) fps = 29.97;
			_waitKey = 1000.0/fps;
		}

		return _waitKey;
	};

	float waitKey( void ) const { return _waitKey; }


protected:

	TCLAP::CmdLine _cmd;
	TCLAP::ValueArg<unsigned int> _skipArg, _strideArg;
	TCLAP::SwitchArg _doDisplayArg;
	TCLAP::ValueArg<int> _waitKeyArg;
	float _waitKey;

	TCLAP::ValueArg<float> _scaleArg;


	TCLAP::ValueArg<string> _videoOutputArg;
	TCLAP::UnlabeledValueArg<string> _videoInputArg;

	fs::path _videoInputPath;


};

class MotionTracking
{
public:
	MotionTracking( MotionTrackingConfig &conf )
		: _conf( conf ),
			_capture( conf.inputFileString() ),
			_opticalFlow( createOptFlow_DualTVL1() )
	{
		cout << "Input file: " << conf.inputFileString() << endl;
}

	~MotionTracking()
	{
		if( _capture.isOpened() ) _capture.release();
	}

	bool operator()( void )
	{
		if( !_capture.isOpened() ) {
			cerr << "Unable to open video file.  Aborting." << endl;
			return false;
		}

		float scale = _conf.scale();

		if( _conf.isVideoOutputSet()) {
			fs::path outputPath( _conf.videoOutput() );
			cout << "Saving flow video to " << outputPath.string() << endl;
			cv::Size sz( frameSize() );
//int( _capture.get( CV_CAP_PROP_FOURCC ))
			_flowVideo.open( outputPath.string(), CV_FOURCC('a','v','c','1'), fps(),
										Size( sz.width*2, sz.height ), true  );

			if( !_flowVideo.isOpened() ) {
				cerr << "Unabled to open output video " << outputPath.string() << ", aborting";
				return false;
			}
		}


		cv::Size sz( frameSize() );
		cv::Size workingSz( sz.width*scale, sz.height*scale );
		cout << "Video frames are " << sz.width << " x " << sz.height << endl;
		cout << "Video is " << frameCount() << " frames long" << endl;
		cout << "    at " << fps() << " fps" << endl;
		cout << "Working size is " << workingSz.width << " x " << workingSz.height << endl;

		_conf.updateWaitKey( fps() );


		if( _conf.skip() > 0 ) {
			for( auto i = 0; i < _conf.skip(); ++i ) {
				if( !_capture.grab() )  {
					cerr << "Reached end of file while skipping." << endl;
					return false;
				}
			}
		}

		Mat current, scaled, prevGray;
		Mat curGray, mag, angle;

		while( _capture.read(current) ) {


			if( _conf.scale() != 1.0 )
				resize( current, scaled, workingSz );
			else
				scaled = current;

			cvtColor( scaled, curGray, CV_BGR2GRAY );

			if( !prevGray.empty() ) {
				Mat flow;
				_opticalFlow->calc( prevGray, curGray, flow );

				std::vector<cv::Mat> components(2);
				split( flow, components );

				cartToPolar( components[0], components[1], mag, angle );
			}

			if( !mag.empty() && (_flowVideo.isOpened() || _conf.doDisplay()) ) {
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

				if( _conf.doDisplay() ) {
					imshow("Magnitude", magRoi );
					imshow("Angle", angleRoi );
				}

				if( _flowVideo.isOpened() )
					_flowVideo.write( composite );
			}

			if( _conf.doDisplay() ) {
				imshow("MotionTracking", current );

				char ch = waitKey( _conf.waitKey() );
				if( ch == 'q' || ch == 'Q') return true;
			}

			prevGray = curGray;

			cout << _capture.get( CV_CAP_PROP_POS_FRAMES ) << endl;

			if( _conf.stride() > 1 ) {
				for( auto i = 0; i < (_conf.stride()-1); ++i ) {
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

 MotionTrackingConfig &_conf;

	VideoCapture _capture;
	VideoWriter _flowVideo;


Ptr<DenseOpticalFlow> _opticalFlow;

};


int main( int argc, char ** argv )
{
	MotionTrackingConfig config( argc, argv );

	MotionTracking mt( config );
	mt();

	exit(0);
}
