#pragma once

#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>

#include <ceres/ceres.h>

#include "frame_processor.h"

namespace CamHDMotionTracking {

  using namespace ceres;

  //=====

  struct SimilarityFunctor {
    SimilarityFunctor( const cv::Mat &delta )
      : _delta( delta ),
        _cx( delta.cols / 2.0 ),
        _cy( delta.rows / 2.0 )
      {;}

    template <typename T>
    bool operator()(const T* const val, T* residual ) const {
      // val[] = {s, theta, tx, ty, cx, cy}

      const T &s( val[0] ),
              &theta( val[1] ),
              &tx( val[2] ),
              &ty( val[3] ),
              &cx( val[4] ),
              &cy( val[5] );

      residual[0] = T(0.0);
      residual[1] = T(0.0);

      //   Just do it manually for now
      // Assumes delta has been reshaped to N x 1
      for( auto i = 0; i < _delta.cols; i++ ) {
        for( auto j = 0; j < _delta.rows; ++j ) {

          auto naught = _delta.at<Vec2f>(j, i );
          const T dx = T(naught[0]),
                  dy = T(naught[1]);

          const T x = T( i ) - cx, y = T( j ) - cy;

          const T cs = ceres::cos( theta ), sn = ceres::sin( theta );

          const T estDx = s * (  cs * x + sn * y ) + tx;
          const T estDy = s * ( -sn * x + cs * y ) + ty;


          residual[0] += estDx - dx;
          residual[1] += estDy - dy;
        }
      }

      return true;
    }

    cv::Mat _delta;
    int _cx, _cy;
  };



  //====




  struct ApproxDerivative : public FrameProcessor {
    ApproxDerivative( const CamHDMovie &mov )
      : FrameProcessor( mov ),
        _flowAlgorithm( cv::createOptFlow_DualTVL1() )
    {;}

    virtual const string jsonName()
    { return "similarity"; }


    virtual json process( int f )
    {
      json stats;
      stats["valid"] = false;

      // Determine the bounds
      const int delta = 1;
      int t1 = f-delta, t2 = f+delta;
      if( f == 1 ) {
        t1 = f;
      } else if( f ==_movie.numFrames() ) {
        t2 = _movie.numFrames();
      }

      if( t1 <= 0 && t2 > _movie.numFrames() ) {
        LOG(WARNING) << "Frame calculation error: t1 = " << t1 << " and t2 = " << t2;
        return stats;
      }

      const float imgScale = 0.25;
      cv::Mat f1( getFrame( t1, imgScale ) ),
              f2( getFrame( t2, imgScale ) );

      if( f1.empty() || f1.empty() ) {
        LOG(WARNING) << "Got an empty frame for t1 = " << t1 << " and t2 = " << t2;
        return stats;
      }

      // Convert to greyscale
      cv::Mat f1Grey, f2Grey;
      cv::cvtColor( f1, f1Grey, CV_RGB2GRAY );
      cv::cvtColor( f2, f2Grey, CV_RGB2GRAY );

      // Do some heuristics
      Scalar mean[2];
      mean[0] = cv::mean( f1 );
      mean[1] = cv::mean( f2 );

      const int darknessThreshold = 5;
      if( mean[0][0] < darknessThreshold || mean[1][0] < darknessThreshold ) {
        return stats;
      }

      cv::Mat flow( f1Grey.size(), CV_32FC2 );
      _flowAlgorithm->calc( f1Grey, f2Grey, flow );

      // Scale flow by dt
      flow /= (t2-t1);

      // Use optical flow to estimate transform
      // Downsample flow using linear interoplation
      cv::Mat scaledFlow;
      const float flowScale = 0.25;
      cv::resize( flow, scaledFlow, cv::Size(), flowScale, flowScale, INTER_LINEAR );

      // Match estimated flow to similarity
      double similarity[6] = {1.0, 0.0, 0.0, 0.0, 0.5*scaledFlow.cols, 0.5*scaledFlow.rows };    // s, theta, tx, ty, cx, cy

      LOG(INFO) << similarity;

      Problem problem;
      CostFunction* cost_function =
            new AutoDiffCostFunction<SimilarityFunctor, 2, 6>(new SimilarityFunctor(scaledFlow));
        problem.AddResidualBlock(cost_function, NULL, similarity);

      Solver::Options options;
      options.linear_solver_type = ceres::DENSE_QR;
      //options.minimizer_progress_to_stdout = true;
      Solver::Summary summary;
      Solve(options, &problem, &summary);

      // std::cout << summary.BriefReport() << "\n";
      // std::cout << "x : " << similarity << "\n";

      const double unscale = 1.0 / (flowScale * imgScale );
      double scaledSim[6] = { similarity[0] * unscale,
                              similarity[1],
                              similarity[2] * unscale,
                              similarity[3] * unscale,
                              similarity[4] * unscale,
                              similarity[5] * unscale };

      stats["imgScale"] = imgScale;
      stats["flowScale"] = flowScale;
      stats["rawSim"] = similarity;
      stats["scaledSim"] = scaledSim;
      stats["valid"] = true;

      return stats;
    }


    Ptr< DenseOpticalFlow > _flowAlgorithm;

  };


}
