#pragma once

#include <fstream>

#include <g3log/g3log.hpp>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "json.hpp"
using json = nlohmann::json;

namespace CamHDMotionTracking {

  enum RegionType {
    undefined,
    Unknown,
    Static,
    ZoomIn,
    ZoomOut
  };

  class Regions {
  public:

    struct Region {
      Region( int _start, int _end, const std::string &t )
      : start(_start), end(_end ), _type( t )
      {;}

      int mean() const { return (start+end)/2; }

      bool is( RegionType t ) const      { return type() == t; }
      RegionType type( void ) const;
      const std::string &typeStr( void ) const       { return _type; }


      int start, end;

    protected:
      std::string _type;
    };


    Regions();

    Regions reverse() const;

    void load( const json &j );

    const Region &operator[](int i) const               { return _regions[i]; }

    std::vector< Region >::iterator begin()             { return _regions.begin(); }
    std::vector< Region >::iterator end()               { return _regions.end(); }

    std::vector< Region >::const_iterator begin() const { return _regions.begin(); }
    std::vector< Region >::const_iterator end() const   { return _regions.end(); }

    std::vector< Region >::const_reverse_iterator rbegin() const { return _regions.rbegin(); }
    std::vector< Region >::const_reverse_iterator rend() const   { return _regions.rend(); }


    void push_back( const Region &region )              { _regions.push_back( region ); }

    size_t size() const                                 { return _regions.size(); }


  protected:

    Regions( const std::vector<Region> &rev );

    std::vector< Region > _regions;

  };

  // Operators on the top-level JSON
  void operator>>( json &j, Regions &region );


}
