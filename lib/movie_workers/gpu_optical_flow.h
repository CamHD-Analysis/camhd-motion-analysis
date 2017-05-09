#pragma once

#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/gpu/gpu.hpp>

#include "frame_processor.h"
#include "optical_flow.h"

namespace CamHDMotionTracking {

  struct GpuOpticalFlow : public OpticalFlow {
    GpuOpticalFlow( const CamHDMovie &mov );

    virtual bool calcFlow( int t1, int t2 );

    //cv::gpu::GpuMat gpuBuildMask( const cv::gpu::GpuMat &grey );

  };

  class GPUOpticalFlowFactory : public FrameProcessorFactory {
  public:

    virtual std::shared_ptr<FrameProcessor> operator()( const CamHDMovie &mov )
    {
      auto flow =  std::shared_ptr<FrameProcessor>(new GpuOpticalFlow( mov ));
      flow->doDisplay = doDisplay;

      return flow;
    }

  };


}
