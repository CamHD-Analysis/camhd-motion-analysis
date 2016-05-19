#include <string>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video.hpp>

#include <opencv2/gpu/gpu.hpp>

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
			_waitKey(-1)
	{}

	~MotionTracking()
	{
		//if( _capture.isOpened() ) _capture.release();
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
		_format = _capture.format();

		if( !_flowVideoPath.empty() > 0 ) {
			cout << "Saving flow video to " << _flowVideoPath.string() << endl;
			cv::Size sz( frameSize() );
			_flowVideo.open( _flowVideoPath.string(), Size( sz.width*2, sz.height ), fps(), gpu::VideoWriter_GPU::SF_BGR  );

			if( !_flowVideo.isOpened() ) {
				cerr << "Unabled to open output video " << _flowVideoPath.string() << ", aborting";
				return false;
			}
		}

		cv::Size sz( frameSize() );
		cout << "Video frames are " << sz.width << " x " << sz.height << endl;

		// Pre-allocate everything
		gpu::GpuMat current( sz, CV_8UC3 ), curGrey( sz, CV_8UC1 ), prevGrey( sz, CV_8UC1 );
		gpu::GpuMat composite( Size( sz.width*2, sz.height), CV_8UC3 );
		gpu::GpuMat flowx( sz, CV_32F ), flowy( sz, CV_32F );
		gpu::GpuMat mag( sz, CV_32F ), angle( sz, CV_32F );

		gpu::GpuMat magInt( sz, CV_8UC1 ), angInt( sz, CV_8UC1 );

		gpu::FarnebackOpticalFlow _opticalFlow;


		if( _skip > 0 ) {
			for( auto i = 0; i < _skip; ++i ) {
				if( !_capture.read(current) )  {
					cerr << "Reached end of file while skipping." << endl;
					return false;
				}
			}
		}

		int frameNum = _skip;
		while( _capture.read(current) ) {

			gpu::cvtColor( current, curGrey, CV_BGR2GRAY );

			if( frameNum > _skip ) {
				_opticalFlow( curGrey, prevGrey, flowx, flowy );
				gpu::cartToPolar( flowx, flowy, mag, angle );

				gpu::GpuMat magRoi( composite, cv::Rect(0,0, sz.width, sz.height ) ),
						   angleRoi( composite, cv::Rect( sz.width, 0, sz.width, sz.height ));

				double magMin, magMax = 1;
				gpu::minMaxLoc( mag, &magMin, &magMax );

				mag.convertTo( magInt, CV_8UC1, 255.0/magMax );
				angle.convertTo( angInt, CV_8UC1, 255.0/2*M_PI );

				gpu::cvtColor( magInt, magRoi, CV_GRAY2BGR, 3 );
				gpu::cvtColor( angInt, angleRoi, CV_GRAY2BGR, 3 );


				_flowVideo.write( composite );
			}

			prevGrey = curGrey;

			if( _stride > 1 ) {
				for( auto i = 0; i < (_stride-1); ++i ) {
					if( !_capture.read(current) ) return true;
				}
			}

			++frameNum;
		}


		return true;
	}

	cv::Size frameSize( void )
	{
		return cv::Size( _format.width, _format.height );
	}

	double frameCount( void )
	{ return -1; }

	double fps( void )
	{ return 29.97; }



protected:

	fs::path _inputFilePath, _flowVideoPath;

	gpu::VideoReader_GPU _capture;
	gpu::VideoReader_GPU::FormatInfo _format;
	gpu::VideoWriter_GPU _flowVideo;

	unsigned int _stride, _skip;
	bool _doDisplay;
	int _waitKey;

//Ptr<DenseOpticalFlow> _opticalFlow;





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

		int id = gpu::getDevice();
		gpu::DeviceInfo info( id );
		cout << "Running on device \"" << info.name() << "\"" << endl;

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
