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
    SimilarityFunctor( const cv::Mat &delta, const cv::Mat &mask = cv::Mat() )
    : _delta( delta ),
    _mask( mask ),
    _cx( delta.cols / 2.0 ),
    _cy( delta.rows / 2.0 )
    {;}

    template <typename T>
    bool operator()(const T* const scale, const T* const rot, const T* const trans, const T* const center, T* residual ) const {
      // val[] = {s, theta, tx, ty, cx, cy}

      const T &s( scale[0] ),
              &theta( rot[0] ),
              &tx( trans[0] ),
              &ty( trans[1] ),
              &cx( center[0] ),
              &cy( center[1] );
      const T cs = ceres::cos( theta ), sn = ceres::sin( theta );

      residual[0] = T(0.0);
      residual[1] = T(0.0);

      LOG(INFO) << s << "; " << theta << "; " << tx << "; " << ty;

      //   Just do it manually for now
      const int gutter = 5;
      for( auto r = gutter; r < (_delta.rows-gutter); ++r ) {
        for( auto c = gutter; c < (_delta.cols-gutter); ++c ) {


          if( !_mask.empty() && _mask.at<uchar>(r,c) == 0 ) continue;

          const T x = T(c) + cx,
                  y = T(r) + cy;

          auto naught = _delta.at<Vec2f>( r, c );
          const T dx = T(naught[0]),
                  dy = T(naught[1]);

          // const T xmeas = x + dx;
          // const T ymeas = y + dy;


          const T dxpred = s * (  cs * x + sn * y ) + tx - x;
          const T dypred = s * ( -sn * x + cs * y ) + ty - y;

          const T errx = dxpred - dx,
                  erry = dypred - dy;

          //LOG(INFO) << errx << " :: " << erry;

          residual[0] += ceres::sqrt(errx*errx);
          residual[1] += ceres::sqrt(erry*erry);
        }
      }

      return true;
    }

    cv::Mat _delta, _mask;
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


    virtual json asJson( int f )
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

      // Do some heuristics (TODO:  This could be moved earlier)
      Scalar mean[2];
      mean[0] = cv::mean( f1 );
      mean[1] = cv::mean( f2 );

      const int darknessThreshold = 5;
      if( mean[0][0] < darknessThreshold || mean[1][0] < darknessThreshold ) {
        return stats;
      }

      // Build mask
      cv::Mat f1Mask( buildMask( f1Grey ) );
      //cv::Mat f2Mask( buildMask( f2Grey ) );


      // Optical flow calculation
      cv::Mat flow( f1Grey.size(), CV_32FC2 );
      _flowAlgorithm->calc( f1Grey, f2Grey, flow );

      // Scale flow by dt
      //flow /= (t2-t1);

      visualizeFlow( flow, f1Grey, f2Grey, f1Mask );


      // Use optical flow to estimate transform
      // Downsample flow using linear interoplation
      cv::Mat scaledFlow;
      const float flowScale = 1.0;
      cv::resize( flow, scaledFlow, cv::Size(), flowScale, flowScale, INTER_LINEAR );

      auto meanFlow = cv::mean( scaledFlow );

      // Initialize similarity
      double similarity[4] = {1.0, 0.0, meanFlow[0], meanFlow[1] };
      double center[2] = { -0.5*scaledFlow.cols, -0.5*scaledFlow.rows };    // s, theta, tx, ty, cx, cy

      Problem problem;
      CostFunction* cost_function = new AutoDiffCostFunction<SimilarityFunctor, 2, 1, 1, 2, 2>(new SimilarityFunctor(scaledFlow, f1Mask ));

      const double HuberThreshold = 200;
      problem.AddResidualBlock(cost_function, NULL, &(similarity[0]), &(similarity[1]), &(similarity[2]), center);

      problem.SetParameterBlockConstant(center);

      problem.SetParameterBlockConstant(&(similarity[1]));  // Fix theta
      //problem.SetParameterBlockConstant(&(similarity[2]));  // Fix translation

      problem.SetParameterLowerBound( similarity, 0, 0.8 );
      problem.SetParameterUpperBound( similarity, 0, 1.2 );

      // Expect very little rotation
      problem.SetParameterLowerBound( &(similarity[1]), 0, -0.1 );
      problem.SetParameterUpperBound( &(similarity[1]), 0,  0.1 );

      //
      problem.SetParameterLowerBound( &(similarity[2]), 0, -0.25*scaledFlow.cols );
      problem.SetParameterUpperBound( &(similarity[2]), 0,  0.25*scaledFlow.cols );

      problem.SetParameterLowerBound( &(similarity[2]), 1, -0.25 *scaledFlow.rows );
      problem.SetParameterUpperBound( &(similarity[2]), 1,  0.25 *scaledFlow.rows );



      Solver::Options options;
      //options.preconditioner_type = ceres::IDENTITY;
      //options.minimizer_type = LINE_SEARCH;
      //options.linear_solver_type = ceres::DENSE_QR;
      options.max_num_iterations = 1000;
      options.minimizer_progress_to_stdout = true;
      //options.check_gradients = true;
      Solver::Summary summary;
      Solve(options, &problem, &summary);

      LOG(INFO) << summary.FullReport();
      LOG(INFO) << "similarity : " << similarity[0] << " " << similarity[1] << " " << similarity[2] << " " << similarity[3];
      LOG(INFO) << "center : " << center[0] << " " << center[1];

      // TODO Tests for validity of solution




      stats["imgScale"] = imgScale;
      stats["flowScale"] = flowScale;

      const double totalScale = flowScale * imgScale;

      // Calculate unscaled translation

      similarity[2] /= totalScale;
      similarity[3] /= totalScale;

      center[0] /= totalScale;
      center[1] /= totalScale;

      // double similarity[6] = { similarity[0],
      //   similarity[1],
      //   similarity[2],
      //   similarity[3] ,
      //   center[0],
      //   center[1]};
      //      double rawSim[6] = { similarity[0], similarity[1], similarity[2], similarity[3], center[0],center[1] };

      //double centerScaled[2] = {center[0] * unscale, center[1] * unscale};

      stats["similarity"] = similarity;
      stats["center"] = center;
      stats["valid"] = true;


      visualizeWarp( full1, full2, similarity, center );

      return stats;
    }


    Ptr< DenseOpticalFlow > _flowAlgorithm;



    void visualizeFlow( const cv::Mat &flow, const cv::Mat &f1, const cv::Mat &f2, const cv::Mat &mask = cv::Mat() )
    {
      std::vector< cv::Mat > channels(2);
      cv::split( flow, channels );

      float m = std::max( cv::norm(channels[0], NORM_INF), cv::norm(channels[1], NORM_INF) );

      LOG(INFO) << "Max x : " << cv::norm(channels[0], NORM_INF) << " : Max y: " << cv::norm(channels[1], NORM_INF);

      cv::Mat xScaled( channels[0]/m );
      cv::Mat xnScaled( -xScaled );

      cv::Mat yScaled( channels[1]/m );
      cv::Mat ynScaled( -yScaled );

      cv::Mat nil( cv::Mat::zeros( xScaled.size(), xScaled.type() ));

      vector<cv::Mat> xchan;
      xchan.push_back( xnScaled ); //  B
      xchan.push_back( nil );      // G
      xchan.push_back( xScaled );  // R

      vector<cv::Mat> ychan;
      ychan.push_back( ynScaled ); // B
      ychan.push_back( nil );
      ychan.push_back( yScaled );  // R

      cv::Mat flowComposite( cv::Mat::zeros(cv::Size(xScaled.size().width + yScaled.size().width, xScaled.size().height), CV_32FC3 ) );
      cv::Mat xroi( flowComposite, cv::Rect(0,0, xScaled.size().width, xScaled.size().height ));
      cv::Mat yroi( flowComposite, cv::Rect(xScaled.size().width,0, yScaled.size().width, yScaled.size().height ));

      if( mask.empty() ) {
        cv::merge( xchan, xroi );

        cv::merge( ychan, yroi );
      } else {
        cv::Mat xflow, yflow;

        cv::merge(xchan,xflow);
        cv::merge(ychan,yflow);

        xflow.copyTo( xroi, mask );
        yflow.copyTo( yroi, mask );

      }

      // imshow( "image 1", f1 );
      // imshow( "image 2", f2 );

      imshow( "flow", flowComposite );

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

      cv::Matx33d warp = cam.inv() * sim * cam;

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



    cv::Mat buildMask( const cv::Mat &grey )
    {
      cv::Mat sobelX, sobelY, mag, angle;

      cv::Sobel( grey, sobelX, CV_32F, 1, 0 );
      cv::Sobel( grey, sobelY, CV_32F, 0, 1 );
      cv::cartToPolar( sobelX, sobelY, mag, angle );

      const float magNorm = std::max( cv::norm( sobelX, NORM_INF ), cv::norm( sobelY, NORM_INF ));

      // imshow( "sobelX", sobelX/magNorm );
      // imshow( "sobelY", sobelY/magNorm );

      // What statistical model for the magnitude



      cv::Mat mask;
      const double magMax( cv::norm( mag, NORM_INF ) );
      const double threshold = 0.1 * magMax;

      cv::compare( mag, threshold, mask, CMP_GT );

      // imshow("mag", mag/magMax );
      // imshow("mask", mask);

      cv::dilate( mask, mask, Mat(), Point(-1,-1), 5 );

      // imshow("dilated", mask);

      // waitKey(0);
      //
      //
      return mask;
    }



  };

}
