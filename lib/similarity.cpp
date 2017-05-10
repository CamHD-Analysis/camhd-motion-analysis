
#include "similarity.h"


namespace CamHDMotionTracking {

      Similarity &Similarity::operator=( const Similarity &other ) {
        scale = other.scale;
        theta = other.theta;
        trans = other.trans;

        return *this;
      }

      Similarity Similarity::operator*( const Similarity &other ) {
        // Probably foolish for rewriting This

        auto cs = cos( other.theta );
        auto sn = sin( other.theta );

        return Similarity( other.scale * scale,
                           other.theta + theta,
                           other.trans[0] + other.scale * (  cs * trans[0] + sn * trans[1]),
                           other.trans[1] + other.scale * ( -sn * trans[0] + cs * trans[1]) );
      }


      cv::Matx23d Similarity::affine( float imgScale) const {
        double cs = cos(theta), sn = sin(theta);
        return cv::Matx23d( scale * cs, scale * sn, imgScale * trans[0],
                           -scale * sn,  scale * cs, imgScale * trans[1] );
      }


      //== Calculated Similarity
      CalculatedSimilarity CalculatedSimilarity::operator*( const Similarity &other ) {
        CalculatedSimilarity newSim;

        // ok, this is sortof ugly
        Similarity foo( scale, theta, trans[0], trans[1] );
        auto res = foo * other;

        newSim.setSimilarity( res.scale, res.theta, res.trans )
              .setCenter( center )
              .setValid( valid )
              .setImageScale( imgScale )
              .setFlowScale( flowScale )
              .setFromFrame( fromFrame )
              .setToFrame( toFrame );

          return newSim;
      }
}
