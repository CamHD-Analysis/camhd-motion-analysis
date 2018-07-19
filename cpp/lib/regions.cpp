

#include "regions.h"


namespace CamHDMotionTracking {

  Regions::Regions()
    : _regions()
    {

    }

  // Private constructor for copying / reversing
  Regions::Regions( const std::vector<Region> &rev  )
    : _regions(rev)
    {
    }



    void Regions::load( const json &j )
    {
      _regions.clear();

      for( auto region : j ) {
        auto bounds = region["bounds"];
        _regions.push_back( Region(bounds[0].get<int>(), bounds[1].get<int>(), region["type"] ) );
      }
    }

    Regions Regions::reverse() const
    {
      std::vector<Region> revRegion;
      std::copy( rbegin(), rend(), std::back_inserter(revRegion) );

      return Regions( revRegion );
    }


  void operator>>( json &j, Regions &region ) {
    // Todo.  Validation of "contents" could go here...

    if( j.find("regions") == j.end() ) return;

    region.load( j["regions"] );
  }


  //===

  RegionType Regions::Region::type( void ) const {
    if( _type == "static" )
      return Static;
    else if( _type == "zoom_in" )
      return ZoomIn;
    else if( _type == "zoom_out" )
      return ZoomOut;
    else if( _type == "unknown" )
      return Unknown;


    LOG(INFO) << "unknown region type \"" << _type << "\"";
    return undefined;
  }

}
