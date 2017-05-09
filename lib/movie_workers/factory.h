#pragma once


namespace CamHDMotionTracking {

  class FrameProcessorFactory {
  public:

    typedef std::unique_ptr<FrameProcessorFactory> Ptr;

    FrameProcessorFactory()
    {;}

    virtual std::shared_ptr<FrameProcessor> operator()( const CamHDMovie &mov ) = 0;

    bool doDisplay;

  };

}
