#pragma once

#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "frame_processor.h"

namespace CamHDMotionTracking {

  struct FrameStatistics : public FrameProcessor {
    FrameStatistics( const CamHDMovie &mov )
    :FrameProcessor( mov )
    {;}

    virtual const string jsonName() 
    { return "statistics"; }

    virtual json process( int f )
    {
      json stats;
      cv::Mat img( getFrame(f) );
      if( img.empty() ) return stats;

      Scalar mean, stdDev;
      cv::meanStdDev( img, mean, stdDev );

      stats["chanMeans"] = mean;
      stats["chanStdDevs"] = stdDev;
      stats["mean"] = cv::mean(mean, Vec4b(1,1,1,0))[0];
      stats["stdDev"] = cv::norm(stdDev, cv::NORM_L2, Vec4b(1,1,1,0));

      return stats;
    }


  };


}
