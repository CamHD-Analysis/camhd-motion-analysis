#pragma once

#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/video/video.hpp>

#include "json.hpp"

#include "frame_processor.h"
#include "similarity.h"

#include "json_contents.h"

#include "factory.h"

namespace CamHDMotionTracking {

  struct OpticalFlow : public FrameProcessor {
    OpticalFlow( const CamHDMovie &mov );

    static const std::string OPTICAL_FLOW_JSON_NAME;
    static const std::string OPTICAL_FLOW_JSON_VERSION;

    const float _imgScale = 0.25;
    const float _flowScale = 1.0;

    cv::Mat _scaledFlow, _scaledMask;
    cv::Mat _full1, _full2;

    virtual const string jsonName();

    virtual bool calcFlow( int t1, int t2 );

    virtual CalculatedSimilarity estimateVelocity( int f );
    virtual CalculatedSimilarity estimateSimilarity( int f, int t );

    virtual nlohmann::json asJson( int f );

    void visualizeFlow( const cv::Mat &flow, const cv::Mat &f1, const cv::Mat &f2, const cv::Mat &mask = cv::Mat() );

    void visualizeWarp( const cv::Mat &f1, const cv::Mat &f2, double *scaledSim, double *center );

    cv::Mat buildMask( const cv::Mat &grey );


  };

  class OpticalFlowFactory : public FrameProcessorFactory {
  public:

    virtual std::shared_ptr<FrameProcessor> operator()( const CamHDMovie &mov )
    {
      auto flow =  std::shared_ptr<FrameProcessor>(new OpticalFlow( mov ));
      flow->doDisplay = doDisplay;

      return flow;
    }

    // Does this requires RTTI?
    virtual void addJSONContents( json &j ) {
      CamHDMotionTracking::addJSONContents( j, OpticalFlow::OPTICAL_FLOW_JSON_NAME, OpticalFlow::OPTICAL_FLOW_JSON_VERSION );
    }

  };




}
