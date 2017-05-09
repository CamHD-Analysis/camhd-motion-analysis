

#include "regions.h"


namespace CamHDMotionTracking {

  // Private constructor for copying / reversing
  Regions::Regions( const std::vector<Region> &rev  )
    : _regions(rev)
    {
    }

  Regions::Regions( const json &j )
    : _regions()
  {
    load( j );
  }

    void Regions::load( const json &j )
    {
      _regions.clear();

      // std::ifstream in( jsonFile.string() );
      // json j;
      // in >> j;

        std::cout << j << std::endl;

      for( auto r : j["regions"] ) {
        _regions.push_back( Region(r[0].get<int>(), r[1].get<int>() ) );
      }
    }

    Regions Regions::reverse() const
    {
      std::vector<Region> revRegion;
      std::copy( rbegin(), rend(), std::back_inserter(revRegion) );

      return Regions( revRegion );
    }


}
