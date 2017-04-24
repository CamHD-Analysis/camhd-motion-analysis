#pragma once

#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/video/video.hpp>

#include "frame_processor.h"

namespace CamHDMotionTracking {

  struct OpticalFlow : public FrameProcessor {
    OpticalFlow( const CamHDMovie &mov );

    const float _imgScale = 0.25;
    const float _flowScale = 1.0;

    cv::Mat _scaledFlow, _scaledMask;
    cv::Mat _full1, _full2;

    virtual const string jsonName();

    virtual bool calcFlow( int t1, int t2 );

    virtual json process( int f );

    void visualizeFlow( const cv::Mat &flow, const cv::Mat &f1, const cv::Mat &f2, const cv::Mat &mask = cv::Mat() );

    void visualizeWarp( const cv::Mat &f1, const cv::Mat &f2, double *scaledSim, double *center );


    cv::Mat buildMask( const cv::Mat &grey );


  };

  inline std::shared_ptr<FrameProcessor> OpticalFlowFactory( const CamHDMovie &mov )
  {
    return std::shared_ptr<FrameProcessor>(new OpticalFlow( mov ));
  }

}
