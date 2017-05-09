#pragma once

#include <opencv2/core/core.hpp>

namespace CamHDMotionTracking {

  using cv::Vec2d;

  struct CalculatedSimilarity {

    CalculatedSimilarity()
    : valid(false)
    {;}

    CalculatedSimilarity &setSimilarity( double sc, double th, const cv::Vec2d &t )
    { scale = sc; theta = th; trans = t; return *this; }

    CalculatedSimilarity &setCenter( const cv::Vec2d &c )
    { center = c; return *this; }

    CalculatedSimilarity &setValid( bool v )
    { valid = v; return *this; }

    CalculatedSimilarity &setImageScale( float s )
    { imgScale = s; return *this; }

    CalculatedSimilarity &setFlowScale( float s )
    { flowScale = s; return *this; }

    CalculatedSimilarity &setFromFrame( int t )
    { fromFrame = t; return *this; }

    CalculatedSimilarity &setToFrame( int t )
    { toFrame = t; return *this; }

    double scale, theta;
    Vec2d trans, center;
    float imgScale, flowScale;
    int fromFrame, toFrame;
    bool valid;

  };

  inline void to_json(json& j, const CalculatedSimilarity& sim) {

    j["imgScale"] = sim.imgScale;
    j["flowScale"] = sim.flowScale;
    j["fromFrame"] = sim.fromFrame;
    j["toFrame"] = sim.toFrame;

    double values[4] = {sim.scale, sim.theta, sim.trans[0], sim.trans[1]};
    j["similarity"] = values;
    double c[2] = {sim.center[0],sim.center[1]};
    j["center"] = c;
    j["valid"] = sim.valid;

  }

}
