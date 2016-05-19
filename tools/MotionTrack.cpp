#include <string>
#include <vector>
#include <array>
#include <memory>

#include <chrono>
#include <numeric>

#include <fstream>

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

bool checkKbd( char ch = 0)
{
	if( ch == 0 && kbhit() ) {
		char ch = fgetc(stdin);
	}

	if( ch == 'q' || ch == 'Q') return true;

	return false;
}


class MotionTrackingConfig {
public:

	MotionTrackingConfig( int argc, char **argv )
		: _cmd("Frame-by-frame motion tracking from CamHD files", ' ', ""),
			_skipArg("","skip","Number of frames",false,0,"Number of frames", _cmd),
			_strideArg("s","stride","Number of frames",false,10,"Number of frames", _cmd),
			_stopArg("","stop","Stop at frame #",false,0,"Number of frames", _cmd),
			_doDisplayArg("x","display","", _cmd, false ),
			_waitKeyArg("","wait-key","Number of frames",false,-1,"Number of frames", _cmd),
			_scaleArg("","scale","Scale",false,1.0,"Scale",_cmd),
			_outputScaleArg("","output-scale","Scale",false,1.0,"Scale",_cmd),
			_blockSizeArg("","block-size","",false,16,"",_cmd),
			_videoOutputArg("","video-out","",false,"","", _cmd),
			_csvOutputArg("","csv-out","",false,"","", _cmd),
			_videoInputArg("input-file", "Input file", true, "", "Input filename", _cmd )
	{
		parse( argc, argv );
	}

	void parse( int argc, char **argv )
	{
		try {
			_cmd.parse( argc, argv );

			fs::path inputPath( videoInputPath() );
			if( !fs::exists(inputPath) || !fs::is_regular_file( inputPath) ) {
				cerr << "error: " << videoInput() << " doesn't exist or isn't readable!" << endl;
				exit(-1);
			}

		} catch (TCLAP::ArgException &e)  // catch any exceptions
		{
			cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
		}

		_waitKey = _waitKeyArg.getValue();
	}

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
	TCLAP::ValueArg<int> _waitKeyArg;
	float _waitKey;

	TCLAP::SwitchArg _doDisplayArg;
	TCLAP::UnlabeledValueArg<string> _videoInputArg;

	#define AUTO_ACCESSOR( type, name ) \
		public: \
			type name( void ) { return _##name##Arg.getValue(); } \
			bool name##Set( void ) { return _##name##Arg.isSet(); }

	#define TCLAP_ACCESSOR( type, name ) \
		protected: \
			TCLAP::ValueArg<type> _##name##Arg; \
		public: \
			AUTO_ACCESSOR( type, name )


		TCLAP_ACCESSOR( unsigned int, skip )
		TCLAP_ACCESSOR( unsigned int, stride )
		TCLAP_ACCESSOR( unsigned int, stop )

		TCLAP_ACCESSOR( unsigned int, blockSize )

		TCLAP_ACCESSOR( float, scale )
		TCLAP_ACCESSOR( float, outputScale )


		AUTO_ACCESSOR( bool, doDisplay )

		AUTO_ACCESSOR( std::string, videoInput )
		fs::path videoInputPath( void ) { return _videoInputArg.getValue(); }

		TCLAP_ACCESSOR( string, videoOutput )
		fs::path videoOutputPath( void ) { return _videoOutputArg.getValue(); }

		TCLAP_ACCESSOR( string, csvOutput )
		fs::path csvOutputPath( void ) { return _csvOutputArg.getValue(); }

};


class MotionTrackingCSV {
public:
	MotionTrackingCSV( fs::path filename )
		: _filename( filename )
	{
			if( !_filename.empty() ) {
				_out.open( _filename.string() );
				writeHeader();
			}
	}

	bool isOpened( void ) const
	{
		if( !_filename.empty() && _out.is_open() ) return true;
		return false;
	}

	void writeHeader()
	{
		if( !isOpened() ) return;

		_out << "# " << endl;

	}

	void write( unsigned int frameNum, std::array<cv::Mat,2>  &blockMeans, Vec2f &overallMean, float scale = 1.0 )
	{
		_out << frameNum << "," << overallMean[0]/scale << "," << overallMean[1]/scale;

		for( Point p(0,0); p.y < blockMeans[0].rows; ++p.y) {
			for( p.x = 0; p.x < blockMeans[0].cols; ++p.x ) {

				float bmx = blockMeans[0].at<float>(p);
				float bmy = blockMeans[1].at<float>(p);

				_out << "," << bmx/scale << "," << bmy/scale;
			}
		}

		_out << endl;

	}

protected:

	fs::path _filename;
	ofstream _out;
};


class MotionTracking
{
public:
	MotionTracking( MotionTrackingConfig &conf )
		: _conf( conf ),
			_csv( conf.csvOutputPath() ),
			_capture( conf.videoInput() ),
			_writer(  ),
			_opticalFlow( createOptFlow_DualTVL1() )
	{
		cout << "Input file: " << conf.videoInput() << endl;
}

	~MotionTracking()
	{
		cout << "Closing video file." << endl;
		if( _capture.isOpened() ) _capture.release();
	}

	bool operator()( void )
	{
		if( !_capture.isOpened() ) {
			cerr << "Unable to open video file.  Aborting." << endl;
			return false;
		}

		float scale = _conf.scale();
		float outScale = scale;
		if( _conf.outputScaleSet() ) outScale = _conf.outputScale();

		cv::Size sz( frameSize() );
		_cropRect = Rect( cv::Point(0,0), sz );

		cv::Size workingSz( sz.width*scale, sz.height*scale );
		// Adjust workingSz for blockSize
		if( workingSz.width % _conf.blockSize() != 0 ) {
			workingSz.width = floor(workingSz.width / _conf.blockSize()) * _conf.blockSize();
			_cropRect.width = floor(workingSz.width / scale);
			_cropRect.x = round((sz.width - _cropRect.width)/2);

			cout << "Adjusting working width to " << workingSz.width << " and image crop size to " << _cropRect.width << endl;
		}
		if( workingSz.height % _conf.blockSize() != 0 ) {
			workingSz.height = floor(workingSz.height / _conf.blockSize()) * _conf.blockSize();
			_cropRect.height = floor(workingSz.height / scale);
			_cropRect.y = round((sz.height - _cropRect.height)/2);

			cout << "Adjusting working height to " << workingSz.height << " and image crop size to " << _cropRect.height << endl;
		}

		cv::Size outputSz( _cropRect.size().width*outScale, _cropRect.size().height*outScale );
		cv::Size compositeSz( 3*outputSz.width, outputSz.height );

		cv::Size blockSz( workingSz.width / _conf.blockSize(), workingSz.height / _conf.blockSize() );
		cout << "Video frames are " << sz.width << " x " << sz.height << endl;
		cout << "Video is " << frameCount() << " frames long" << endl;
		cout << "    at " << fps() << " fps" << endl;
		cout << "Working size is " << workingSz.width << " x " << workingSz.height << endl;

		if( _conf.videoOutputSet()) {
			fs::path outputPath( _conf.videoOutput() );
			cout << "Saving flow video to " << outputPath.string() << endl;
			cv::Size sz( frameSize() );
//int( _capture.get( CV_CAP_PROP_FOURCC ))
			float fpsOut = fps()/10;
			cout << " fpsOut: " << fpsOut << endl;
			_writer.reset( new VideoWriter( outputPath.string(), CV_FOURCC('a','v','c','1'), fpsOut,
										compositeSz, true  ) );

			if( !_writer->isOpened() ) {
				cerr << "Unabled to open output video " << outputPath.string() << ", aborting";
				return false;
			}
		}


		_conf.updateWaitKey( fps() );


		if( _conf.skip() > 0 ) {
			_capture.set(CV_CAP_PROP_POS_FRAMES, _conf.skip() );
			// for( auto i = 0; i < _conf.skip(); ++i ) {
			// 	if( !_capture.grab() )  {
			// 		cerr << "Reached end of file while skipping." << endl;
			// 		return false;
			// 	}
			// }
		}

		int frameNum = _capture.get( CV_CAP_PROP_POS_FRAMES );

		Mat current, prevGray, scaled;

		vector<  std::chrono::system_clock::duration > dts;

		cout << "Press 'q' to interrupt (be sure an OpenCV window has focus)" << endl;

		auto start = std::chrono::system_clock::now();

		while( _capture.read(current) ) {
			// curGray must be inside the loop otherwise
			// prevGray = curGray doesn't do anything...
			Mat cropped( current, _cropRect );
			Mat curGray, mag, angle;

			if( _conf.scale() != 1.0 )
				resize( cropped, scaled, workingSz );
			else
				scaled = current;

			cvtColor( scaled, curGray, CV_BGR2GRAY );

			if( !prevGray.empty() ) {
				Mat flow;
				_opticalFlow->calc( prevGray, curGray, flow );

				std::vector<cv::Mat> components(2);
				split( flow, components );

				cartToPolar( components[0], components[1], mag, angle );

				// Resizing by an integer divisor of components[] with INTER_AREA
				// should be equivalent to block-averaging
				std::array<cv::Mat,2> blockMeans;
				resize( components[0], blockMeans[0], blockSz, INTER_AREA );
				resize( components[1], blockMeans[1], blockSz, INTER_AREA );

				Vec2f overallMean( mean( blockMeans[0])[0], mean( blockMeans[1])[0] );

				Mat composite( compositeSz, CV_8UC3 );
				Mat   originalRoi( composite, cv::Rect(0,0, outputSz.width, outputSz.height ) ),
						magRoi( composite, cv::Rect(outputSz.width,0, outputSz.width, outputSz.height ) ),
						angleRoi( composite, cv::Rect( 2*outputSz.width, 0, outputSz.width, outputSz.height ));

				double magMin = 0, magMax = 1;
				minMaxLoc( mag, &magMin, &magMax );

				// Normalize for display
				Mat magInt, angInt;
				mag.convertTo( magInt, CV_8UC1, 255.0/magMax );
				angle.convertTo( angInt, CV_8UC1, 255.0/(2*M_PI) );

				if( _conf.outputScaleSet() ) {
					Mat magBGR, angBGR;
					cvtColor( magInt, magBGR, CV_GRAY2BGR );
					cvtColor( angInt, angBGR, CV_GRAY2BGR );

					resize( magBGR, magRoi, outputSz );
					resize( angBGR, angleRoi, outputSz );
				} else {
					cvtColor( magInt, magRoi, CV_GRAY2BGR );
					cvtColor( angInt, angleRoi, CV_GRAY2BGR );
				}
				resize(cropped, originalRoi, outputSz );

				// Overlay motion arrows
				drawArrows( originalRoi, blockMeans, overallMean );
				_csv.write( frameNum, blockMeans, overallMean, _conf.scale()*_conf.skip() );

				if( _writer.get() )
					_writer->write( composite );

				char ch = 0;
				if( _conf.doDisplay() ) {
					imshow("Magnitude", magRoi );
					imshow("Angle", angleRoi );
					imshow("MotionTracking", originalRoi );

					ch = waitKey( _conf.waitKey() );
				}

				if( checkKbd( ch ) ) break;
			}

//			curGray.copyTo( prevGrey );
			prevGray = curGray;

			auto end = std::chrono::system_clock::now();
			dts.push_back( end-start );

			if( frameNum % 100 == 0 ){
				auto meanDt = std::accumulate( dts.begin(), dts.end(), std::chrono::system_clock::duration(0) );

				cout << frameNum << "  mean ms per frame " << std::chrono::duration_cast<std::chrono::milliseconds>(meanDt).count()/dts.size() << endl;
			}

			start = std::chrono::system_clock::now();

			if( _conf.stride() > 1 ) {
				for( auto i = 0; i < (_conf.stride()-1); ++i ) {
					if( !_capture.grab() || checkKbd() ) break;
				}
			}

			if( _conf.stopSet() && frameNum >= _conf.stop() ) break;

			// Returns the number of the _next_ frame to be read
		 frameNum = _capture.get( CV_CAP_PROP_POS_FRAMES );

		}

		cout << "Releasing writer" << endl;
		_writer.release();

		return true;
	}

	void drawArrows( Mat &original, std::array<cv::Mat,2>  &blockMeans, Vec2f &overallMean )
	{
		float scale = _conf.outputScale() / _conf.scale();


		for( Point p(0,0); p.y < blockMeans[0].rows; ++p.y) {
			for( p.x = 0; p.x < blockMeans[0].cols; ++p.x ) {
				// Compute block center
				Point2f center( ((p.x+0.5)*_conf.blockSize()) * scale,
											  ((p.y+0.5)*_conf.blockSize()) * scale );

				float bmx = blockMeans[0].at<float>(p);
				float bmy = blockMeans[1].at<float>(p);
				Point2f offset( bmx * scale,
											  bmy * scale );

				arrowedLine( original, center, center+offset, cv::Scalar(0,0,255), 2 );

			}
		}

		Point2f imgCenter( original.cols / 2, original.rows / 2 );
		Point2f overall( overallMean[0] * scale, overallMean[1] * scale );

		arrowedLine( original, imgCenter, imgCenter+overall, cv::Scalar(0,255,0), 4 );

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
	MotionTrackingCSV _csv;

	VideoCapture _capture;
	std::unique_ptr<VideoWriter> _writer;

	Ptr<DenseOpticalFlow> _opticalFlow;

	cv::Rect _cropRect;

};


int main( int argc, char ** argv )
{
	MotionTrackingConfig config( argc, argv );

	MotionTracking mt( config );
	mt();

	exit(0);
}
