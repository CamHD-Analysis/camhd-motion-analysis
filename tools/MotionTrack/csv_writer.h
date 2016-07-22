#pragma once

#include <opencv2/core.hpp>

#include <boost/filesystem.hpp>

namespace CamHD_MotionTracking {

	namespace fs = boost::filesystem;
	using namespace std;
	using namespace cv;

	class CSVWriter {
	public:
		CSVWriter( fs::path filename )
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

			_out << "# framenum,ms_per_frame,mean_x,mean_y" << endl;

		}

		void write( unsigned int frameNum, float dt, std::array<cv::Mat,2>  &blockMeans, Vec2f &overallMean, float scale = 1.0 )
		{
			_out << frameNum << "," << dt << "," << overallMean[0]/scale << "," << overallMean[1]/scale;

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

}
