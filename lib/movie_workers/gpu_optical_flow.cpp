#include "gpu_optical_flow.h"

#include <opencv2/gpu/gpu.hpp>

#include <glog/logging.h>

namespace CamHDMotionTracking {

  using std::string;

  using namespace cv;

  //====

    GpuOpticalFlow::GpuOpticalFlow( const CamHDMovie &mov )
    : OpticalFlow( mov )
    {;}


    bool GpuOpticalFlow::calcFlow( int t1, int t2 )
    {
      if( t1 <= 0 && t2 > _movie.numFrames() ) {
        LOG(WARNING) << "Frame calculation error: t1 = " << t1 << " and t2 = " << t2;
        return false;
      }

      // Do this for now until we can serialize straight the GPU...
      _full1 = getFrame( t1, 1.0 );
      _full2 = getFrame( t2, 1.0 );

      if( _full1.empty() || _full2.empty() ) {
        LOG(WARNING) << "Got an empty frame for t1 = " << t1 << " and t2 = " << t2;
        return false;
      }

      LOG(INFO) << "Uploading frame to GPU";
      // Do it synchronous for now...
      gpu::GpuMat gFull1, gFull2;
      gFull1.upload( _full1 );
      gFull2.upload( _full2 );

      gpu::GpuMat gResize1, gResize2;

      gpu::resize( gFull1, gResize1, Size(), _imgScale, _imgScale );
      resize( gFull2, gResize2, Size(), _imgScale, _imgScale );

      // Convert to greyscale
      gpu::GpuMat gGrey1, gGrey2;
      gpu::cvtColor( gResize1, gGrey1, CV_RGB2GRAY );
      gpu::cvtColor( gResize2, gGrey2, CV_RGB2GRAY );

      // Do some heuristics (TODO:  This could be moved earlier)
      Scalar mean[2], stdDev[2];
      gpu::meanStdDev( gGrey1, mean[0], stdDev[0] );
      gpu::meanStdDev( gGrey2, mean[1], stdDev[1] );

      const int darknessThreshold = 5;
      if( mean[0][0] < darknessThreshold || mean[1][0] < darknessThreshold ) {
        return false;
      }

      // TODO: Do optical flow on CPU for now
      Mat cGrey1, cGrey2;
      gGrey1.download( cGrey1 );
      gGrey2.download( cGrey2 );

      // Build mask
      Mat cMask1( buildMask( cGrey1 ) );

      // Optical flow calculation
      Mat flow( cGrey1.size(), CV_32FC2 );
      auto flowAlgorithm( cv::createOptFlow_DualTVL1() );
      flowAlgorithm->calc( cGrey1, cGrey2, flow );

      // Scale flow by dt
      //flow /= (t2-t1);

      //visualizeFlow( flow, f1Grey, f2Grey, f1Mask );


      // Use optical flow to estimate transform
      // Downsample flow using linear interoplation
      resize(flow, _scaledFlow, Size(), _flowScale, _flowScale, INTER_LINEAR );
      resize( cMask1, _scaledMask, _scaledFlow.size() );

      // Scale flow values as well.
      _scaledFlow *= _flowScale;

      return true;
    }
\


}
