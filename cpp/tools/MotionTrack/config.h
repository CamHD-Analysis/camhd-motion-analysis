#pragma once

#include <string>

#include <tclap/CmdLine.h>

#include <boost/filesystem.hpp>

namespace CamHD_MotionTracking {

	using namespace std;
	namespace fs = boost::filesystem;

	class Config {
	public:

		Config( int argc, char **argv )
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

}
