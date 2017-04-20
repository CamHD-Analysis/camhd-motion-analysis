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
    bool operator()(const T* const val, const T* const center, T* residual ) const {
      // val[] = {s, theta, tx, ty, cx, cy}

      const T &s( val[0] ),
              &theta( val[1] ),
              &tx( val[2] ),
              &ty( val[3] ),
              &cx( center[0] ),
              &cy( center[1] );

      residual[0] = T(0.0);
      residual[1] = T(0.0);

      //   Just do it manually for now
      const int buffer = 5;
      for( auto j = buffer; j < (_delta.rows-buffer); ++j ) {
      for( auto i = buffer; i < (_delta.cols-buffer); ++i ) {

          auto naught = _delta.at<Vec2f>( j, i );
          const T dx = T(naught[0]),
                  dy = T(naught[1]);

          // Work in Cartesian (not image) coordinates
          const T x = T(i) - cx,
                  y = T(j) - cy;

          const T cs = ceres::cos( theta ), sn = ceres::sin( theta );

          const T xpred = s * (  cs * x + sn * y ) + tx;
          const T ypred = s * ( -sn * x + cs * y ) + ty;

          const T xmeas = x + dx;
          const T ymeas = y + dy;

          residual[0] += (xpred - xmeas)*(xpred - xmeas);
          residual[1] += (ypred - ymeas)*(ypred - ymeas);
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
      const int delta = 5;
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

      cv::Mat full1( getFrame( t1, 1.0 ) ),
                full2( getFrame( t2, 1.0 ) );

      const float imgScale = 0.25;

      cv::Mat f1, f2;
      cv::resize( full1, f1, cv::Size(), imgScale, imgScale );
      cv::resize( full2, f2, cv::Size(), imgScale, imgScale );

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
      //flow /= (t2-t1);

      //visualizeFlow( flow, f1Grey, f2Grey );

      auto meanFlow = cv::mean( flow );

      // Use optical flow to estimate transform
      // Downsample flow using linear interoplation
      cv::Mat scaledFlow;
      const float flowScale = 0.25;
      cv::resize( flow, scaledFlow, cv::Size(), flowScale, flowScale, INTER_LINEAR );

      // Match estimated flow to similarity
      double similarity[4] = {1.0, 0.0, meanFlow[0], meanFlow[1] };
      double center[2] = { 0.5*scaledFlow.cols, 0.5*scaledFlow.rows };    // s, theta, tx, ty, cx, cy

      Problem problem;
      CostFunction* cost_function = new AutoDiffCostFunction<SimilarityFunctor, 2, 4, 2>(new SimilarityFunctor(scaledFlow));

      const double HuberThreshold = 2.0;
      problem.AddResidualBlock(cost_function, new ceres::HuberLoss(HuberThreshold), similarity, center);

      problem.SetParameterBlockConstant(center);

      Solver::Options options;
      options.minimizer_type = LINE_SEARCH;
      options.linear_solver_type = ceres::DENSE_QR;
      options.max_num_iterations = 1000;
      options.minimizer_progress_to_stdout = true;
      Solver::Summary summary;
      Solve(options, &problem, &summary);

      LOG(INFO) << summary.FullReport();
      LOG(INFO) << "similarity : " << similarity[0] << " " << similarity[1] << " " << similarity[2] << " " << similarity[3];
      LOG(INFO) << "center : " << center[0] << " " << center[1];

      stats["imgScale"] = imgScale;
      stats["flowScale"] = flowScale;

      const double unscale = 1.0 / (flowScale * imgScale );

      // Calculate unscaled translation


      // double similarity[6] = { similarity[0],
      //   similarity[1],
      //   similarity[2],
      //   similarity[3] ,
      //   center[0],
      //   center[1]};
//      double rawSim[6] = { similarity[0], similarity[1], similarity[2], similarity[3], center[0],center[1] };

       double centerScaled[2] = {center[0] * unscale, center[1] * unscale};

        stats["similarity"] = similarity;
        stats["center"] = centerScaled;
        stats["valid"] = true;


        visualizeWarp( full1, full2, similarity, centerScaled );

        return stats;
      }


      Ptr< DenseOpticalFlow > _flowAlgorithm;



      void visualizeFlow( const cv::Mat &flow, const cv::Mat &f1, const cv::Mat &f2 )
      {
        std::vector< cv::Mat > channels(2);
        cv::split( flow, channels );

        float m = std::max( cv::norm(channels[0], NORM_INF), cv::norm(channels[1], NORM_INF) );

        LOG(INFO) << "Max x : " << cv::norm(channels[0], NORM_INF) << " : Max y: " << cv::norm(channels[1], NORM_INF);

        imshow( "image 1", f1 );
        imshow( "image 2", f2 );
        imshow( "X flow", channels[0]/m  );
        imshow( "Y flow", channels[1]/m );
        waitKey(0);

      }

      void visualizeWarp( const cv::Mat &f1, const cv::Mat &f2, double *scaledSim, double *center )
      {
        const double s( scaledSim[0] ),
                    theta( scaledSim[1] ),
                    tx( scaledSim[2] ),
                    ty( scaledSim[3] ),
                    cx( center[0] ),
                    cy( center[1] );
        const double cs = std::cos( theta ), sn = std::sin(theta );

        cv::Matx33d sim( s * cs, s * sn, tx, s * -sn, s * cs, ty, 0, 0, 1 );
        cv::Matx33d cam( 1, 0, cx, 0, 1, cy, 0, 0, 1 );

        cv::Matx33d warp = cam * sim * cam.inv();

        LOG(INFO) << "cam:" << cam;
        LOG(INFO) << "cam inv:" << cam.inv();
        LOG(INFO) << "sim:" << sim;
        LOG(INFO) << "warp:" << warp;

        cv::Mat f1warp;
        cv::warpPerspective( f1, f1warp, warp, f1.size() );

        // Build a composite
        const float scale = 0.5;
        const int width = f1.size().width * scale, height = f1.size().height * scale;
        cv::Mat composite( cv::Size(width*3, height), CV_8UC3 );
        cv::Mat roiOne( composite, cv::Rect(0,0,width,height) );
        cv::resize( f1, roiOne, roiOne.size() );

        cv::Mat roiOneWarp( composite, cv::Rect(width,0,width,height));
        cv::resize( f1warp, roiOneWarp, roiOneWarp.size() );

        cv::Mat roiTwo( composite, cv::Rect( 2*width,0, width,height ));
        cv::resize( f2, roiTwo, roiTwo.size() );


imshow( "composite", composite);
waitKey(0);
      }

    };

  }
