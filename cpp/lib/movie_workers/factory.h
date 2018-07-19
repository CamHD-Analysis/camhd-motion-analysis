#pragma once


namespace CamHDMotionTracking {

  class FrameProcessorFactory {
  public:

    typedef std::shared_ptr<FrameProcessorFactory> Ptr;

    FrameProcessorFactory()
    {;}

    virtual std::shared_ptr<FrameProcessor> operator()( const CamHDMovie &mov ) = 0;

    virtual void addJSONContents( json &j ) {;}

    bool doDisplay;

  };

}
