#pragma once

#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/video/video.hpp>

#include "frame_processor.h"

namespace CamHDMotionTracking {

  struct OpticalFlow : public FrameProcessor {
    OpticalFlow( const CamHDMovie &mov );

    Ptr< DenseOpticalFlow > _flowAlgorithm;

    const float _imgScale = 0.25;
    const float _flowScale = 1.0;

    int _t1, _t2;
    cv::Mat _full1, _full2;
    cv::Mat _resize1, _resize2;
    cv::Mat _grey1, _grey2, _mask1;
    cv::Mat _scaledFlow, _scaledMask;

    virtual const string jsonName();

    virtual bool calcFlow( );


    virtual json process( int f );


    void visualizeFlow( const cv::Mat &flow, const cv::Mat &f1, const cv::Mat &f2, const cv::Mat &mask = cv::Mat() );

    void visualizeWarp( const cv::Mat &f1, const cv::Mat &f2, double *scaledSim, double *center );


    cv::Mat buildMask( const cv::Mat &grey );


  };

  std::shared_ptr<FrameProcessor> OpticalFlowFactory( const CamHDMovie &mov )
  {
    return std::shared_ptr<FrameProcessor>(new OpticalFlow( mov ));
  }

}
