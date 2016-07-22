#pragma once

#include <opencv2/core.hpp>

namespace CamHD_MotionTracking {

  using cv::Mat;

  class Frame {
  public:

    delete Frame(void);
    delete Frame( const Frame & );

    Frame( const Mat &img )
      : _img( img )
      {;}

    protected:

      Mat _img;

  };

}
