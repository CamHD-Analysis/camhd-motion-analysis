
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
                           other.trans[0] + other.scale * ( cs * trans[0] + sn * trans[1]),
                           other.trans[1] + other.scale * ( -sn * trans[0] + cs*trans[1]) );
      }

}
