#pragma once

#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "frame_processor.h"

#include "json.hpp"
using json = nlohmann::json;

namespace CamHDMotionTracking {


  struct FrameMean : public FrameProcessor {
    FrameMean( const CamHDMovie &mov )
    : FrameProcessor(mov)
    {;}

    json process( int f ) {
      return json();
    }

    // // Defines a "soft equal"
    // bool operator()( int a, int b )
    // { return compare( a, b ); }

  };


}
