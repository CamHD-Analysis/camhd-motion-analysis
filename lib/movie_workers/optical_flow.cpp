#include "optical_flow.h"

#include <string>

#include <g3log/g3log.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>

#include <ceres/ceres.h>


namespace CamHDMotionTracking {

  using namespace ceres;
  using std::string;

  //=====

  const std::string OpticalFlow::OPTICAL_FLOW_JSON_NAME = "opticalFlow";
  const std::string OpticalFlow::OPTICAL_FLOW_JSON_VERSION = "1.0";

  struct OpticalFlowFunctor {
    OpticalFlowFunctor( const cv::Vec2f &delta, const cv::Point &pt )
    : _delta(delta), _pt( pt )
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

      const T x = T(_pt.x) + cx,
      y = T(_pt.y) + cy;

      const T dxpred = s * (  cs * x + sn * y ) + tx - x;
      const T dypred = s * ( -sn * x + cs * y ) + ty - y;

      const T errx = dxpred - T(_delta[0]),
      erry = dypred - T(_delta[1]);


      //       if( _pt.x == 100 & _pt.y == 250 ) {
      //         LOG(INFO) << "xy: " << x << " :: " << y;
      //         LOG(INFO) << "delta: " << _delta[0] << " :: " <<  _delta[1];
      //         LOG(INFO) << "pred: " << dxpred << " :: " << dypred;
      //         LOG(INFO) << "err: " << errx << " :: " << erry;
      // }

      residual[0] = errx;
      residual[1] = erry;

      return true;
    }

    cv::Vec2f _delta;
    cv::Point _pt;
  };


  //



  OpticalFlow::OpticalFlow( const CamHDMovie &mov )
  : FrameProcessor( mov )
  {;}

  const string OpticalFlow::jsonName()
  { return OPTICAL_FLOW_JSON_NAME; }


  bool OpticalFlow::calcFlow( int t1, int t2, const Similarity &hint )
  {
    if( t1 <= 0 && t2 > _movie.numFrames() ) {
      LOG(WARNING) << "Frame calculation error: t1 = " << t1 << " and t2 = " << t2;
      return false;
    }

    {
      _timerFrame1.start();
      _full1 = getFrame(t1, 1.0);
      _timerFrame1.stop();
      LOG(WARNING) << "Retrieving 1 required " << _timerFrame1.seconds() << " s";
    }

    {
      _timerFrame2.start();
      _full2 = getFrame(t2, 1.0);
      _timerFrame2.stop();
      LOG(WARNING) << "Retrieving 2 required " << _timerFrame2.seconds() << " s";
    }



    if( _full1.empty() || _full2.empty() ) {
      LOG(WARNING) << "Got an empty frame for t1 = " << t1 << " and t2 = " << t2;
      return false;
    }

    // Apply the hint
    cv::Mat warped1;
    cv::warpAffine( _full1, warped1, hint.affine(), _full1.size() );

    cv::Mat resize1, resize2;
    cv::resize( warped1, resize1, cv::Size(), _imgScale, _imgScale );
    cv::resize( _full2, resize2, cv::Size(), _imgScale, _imgScale );

    // Convert to greyscale
    cv::Mat grey1, grey2;
    cv::cvtColor( resize1, grey1, CV_RGB2GRAY );
    cv::cvtColor( resize2, grey2, CV_RGB2GRAY );

    // Do some heuristic tests
    Scalar mean[2];
    mean[0] = cv::mean( grey1 );
    mean[1] = cv::mean( grey2 );

    const int darknessThreshold = 5;
    if( mean[0][0] < darknessThreshold || mean[1][0] < darknessThreshold ) {
      return false;
    }

    // Build mask
    cv::Mat mask( buildMask( grey1  ) );
    cv::Mat warpedMask;
    cv::warpAffine( mask, warpedMask, hint.affine( _imgScale ), mask.size() );



    // Optical flow calculation
    cv::Mat flow( grey1.size(), CV_32FC2 );

    {
      _timerFlow.start();
    auto flowAlgorithm( cv::createOptFlow_DualTVL1() );
    flowAlgorithm->calc( grey1, grey2, flow );

_timerFlow.stop();
    LOG(WARNING) << "Flow calc required " << _timerFlow.seconds() << " s";
  }

    // Scale flow by dt
    //flow /= (t2-t1);

    if( doDisplay ) visualizeFlow( flow, grey1, grey2, warpedMask );

    // Use optical flow to estimate transform
    // Downsample flow using linear interoplation
    cv::resize(flow, _scaledFlow, cv::Size(), _flowScale, _flowScale, INTER_LINEAR );
    cv::resize(warpedMask, _scaledMask, _scaledFlow.size() );

    // Scale flow values as well.
    _scaledFlow *= _flowScale;

    return true;
  }

  json OpticalFlow::asJson( int f )
  {
    // Implicit conversion from CalculatedSimilarity
    json j = estimateVelocity(f);

    json jtiming;
    jtiming["retrieveFrame1"] = _timerFrame1.seconds();
    jtiming["retrieveFrame2"] = _timerFrame2.seconds();
    jtiming["opticalFlow"] = _timerFlow.seconds();
    jtiming["minimization"] = _timerMinimize.seconds();
    jtiming["all"] = _timerFullCallback.seconds();

    json jperf;
    jperf["timing"] = jtiming;

    j["performance"] = jperf;

    return j;
  }

  CalculatedSimilarity OpticalFlow::estimateVelocity( int f )
  {

    // Determine the bounds
    const int delta = 5;
    int t1 = f-delta, t2 = f+delta;
    if( f == 1 ) {
      t1 = f;
    } else if( f ==_movie.numFrames() ) {
      t2 = _movie.numFrames();
    }

    return estimateSimilarity( t1, t2 );
  }

  CalculatedSimilarity OpticalFlow::estimateSimilarity( int t1, int t2, const Similarity &hint )
  {
    _timerFullCallback.start();

    if( !calcFlow( t1, t2, hint) ) return CalculatedSimilarity();

    auto meanFlow = cv::mean( _scaledFlow );

    // Initialize similarity
    double similarity[4] = {1.0, 0.0, meanFlow[0], meanFlow[1] };
    double center[2] = { -0.5*_scaledFlow.cols, -0.5*_scaledFlow.rows };    // s, theta, tx, ty, cx, cy

    Problem problem;

    const double HuberThreshold = 200;

    int count = 0;
    const int gutter = 5;
    for( auto r = gutter; r < (_scaledFlow.rows-gutter); ++r ) {
      for( auto c = gutter; c < (_scaledFlow.cols-gutter); ++c ) {

        if( !_scaledMask.empty() && _scaledMask.at<uchar>(r,c) == 0 ) continue;

        CostFunction* cost_function = new AutoDiffCostFunction<OpticalFlowFunctor, 2, 1, 1, 2, 2>(new OpticalFlowFunctor( _scaledFlow.at<Vec2f>(r,c), cv::Point(c,r) ));
        problem.AddResidualBlock(cost_function, NULL, &(similarity[0]), &(similarity[1]), &(similarity[2]), center);
        ++count;
      }
    }


    LOG(INFO) << "Used " << count << " of " << (_scaledFlow.rows-2*gutter)*(_scaledFlow.cols-2*gutter) << " flow points";

    problem.SetParameterBlockConstant(&(similarity[1]));  // Fix theta
    problem.SetParameterBlockConstant(&(similarity[2]));  // Fix translation
    problem.SetParameterBlockConstant(center);

    problem.SetParameterLowerBound( similarity, 0, 0.5 );
    problem.SetParameterUpperBound( similarity, 0, 1.5 );

    // Expect very little rotation
    problem.SetParameterLowerBound( &(similarity[1]), 0, -0.1 );
    problem.SetParameterUpperBound( &(similarity[1]), 0,  0.1 );

    //
    problem.SetParameterLowerBound( &(similarity[2]), 0, -0.25*_scaledFlow.cols );
    problem.SetParameterUpperBound( &(similarity[2]), 0,  0.25*_scaledFlow.cols );

    problem.SetParameterLowerBound( &(similarity[2]), 1, -0.25 * _scaledFlow.rows );
    problem.SetParameterUpperBound( &(similarity[2]), 1,  0.25 * _scaledFlow.rows );


{
      _timerMinimize.start();

    Solver::Options options;
    //options.preconditioner_type = ceres::IDENTITY;
    //options.minimizer_type = LINE_SEARCH;
    options.linear_solver_type = ceres::DENSE_QR;
    options.max_num_iterations = 1000;
    options.minimizer_progress_to_stdout = true;
    //options.check_gradients = true;
    Solver::Summary summary;
    Solve(options, &problem, &summary);

    LOG(INFO) << summary.FullReport();
    LOG(INFO) << "similarity : " << similarity[0] << " " << similarity[1] << " " << similarity[2] << " " << similarity[3];
    LOG(INFO) << "center : " << center[0] << " " << center[1];

    // TODO Tests for validity of solution

    _timerMinimize.stop();
    LOG(WARNING) << "Ceres required " << _timerMinimize.seconds() << " s";
  }


    const double totalScale = _flowScale * _imgScale;

    // Calculate unscaled translation

    similarity[2] /= totalScale;
    similarity[3] /= totalScale;

    center[0] /= totalScale;
    center[1] /= totalScale;

    CalculatedSimilarity sim;
    sim.setSimilarity( similarity[0], similarity[1], cv::Vec2d( similarity[2], similarity[3]))
    .setCenter( cv::Vec2d(center[0],center[1]) )
    .setImageScale( _imgScale )
    .setFlowScale( _flowScale )
    .setFromFrame( t1 )
    .setToFrame( t2 )
    .setValid( true );

    auto finalSim = sim*hint;

    if( doDisplay ) {
      double foobar[] = {finalSim.scale, finalSim.theta, finalSim.trans[0], finalSim.trans[1]};
      visualizeWarp( _full1, _full2, foobar, center );
      waitKey(1);
    }

    _timerFullCallback.stop();
    LOG(WARNING) << "Everything required " << _timerFullCallback.seconds() << " s";

    return finalSim;
  }



  void OpticalFlow::visualizeFlow( const cv::Mat &flow, const cv::Mat &f1, const cv::Mat &f2, const cv::Mat &mask )
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

  void OpticalFlow::visualizeWarp( const cv::Mat &f1, const cv::Mat &f2, double *scaledSim, double *center )
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
    //  waitKey(1);
  }



  cv::Mat OpticalFlow::buildMask( const cv::Mat &grey )
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

}
