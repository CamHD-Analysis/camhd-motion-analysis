#pragma once

#include <ostream>

#include <opencv2/core/core.hpp>

#include "json.hpp"
using json = nlohmann::json;

namespace CamHDMotionTracking {

  using cv::Vec2d;

  static const std::string SIMILARITY_JSON_NAME = "similarity";
  static const std::string SIMILARITY_JSON_VERISON = "1.0";

  struct Similarity {

    Similarity( double sc = 1.0, double th = 0.0, double tx = 0.0, double ty = 0.0 )
    : scale(sc), theta(th), trans( Vec2d(tx,ty) )
    {;}

    double scale, theta;
    Vec2d trans;

    Similarity &operator=( const Similarity &other );
    Similarity operator*( const Similarity &other );

    cv::Matx23d affine( float imgScale = 1.0 ) const;

  };

  struct CalculatedSimilarity : public Similarity {

    CalculatedSimilarity()
    : center( Vec2d(0,0) ),
      imgScale(0.0), flowScale(0.0),
      fromFrame(0), toFrame(0),
      valid(false)
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

    // As above for Similarity but retains metadata
    CalculatedSimilarity operator*( const Similarity &other );

    Vec2d center;
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

  inline void operator<<( std::ostream &out, const Similarity &sim ) {
    out << cv::Vec4d( sim.scale, sim.theta, sim.trans[0], sim.trans[1]);
  }

}
