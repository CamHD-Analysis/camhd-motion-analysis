

#include "MotionTrack/MotionTracking.h"

#include "g3log/g3log.hpp"

#include "kbhit.h"

using namespace std;
using namespace cv;

using namespace CamHD_MotionTracking;

int main( int argc, char ** argv )
{
	CamHD_MotionTracking::Config config( argc, argv );

	MotionTracking mt( config );
	mt();

	exit(0);
}
