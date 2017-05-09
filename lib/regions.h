#pragma once

#include <fstream>

#include <g3log/g3log.hpp>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "json.hpp"
using json = nlohmann::json;

namespace CamHDMotionTracking {

  class Regions {
  public:

    struct Region {
      Region( int _start, int _end )
      : start(_start), end(_end )
      {;}

      int mean() const { return (start+end)/2; }

      int start, end;
    };


    Regions( const json &j );

    Regions reverse() const;

    void load( const json &j );

    const Region &operator[](int i) const { return _regions[i]; }

    std::vector< Region >::iterator begin() { return _regions.begin(); }
    std::vector< Region >::iterator end()    { return _regions.end(); }

    std::vector< Region >::const_iterator begin() const { return _regions.begin(); }
    std::vector< Region >::const_iterator end() const   { return _regions.end(); }

    std::vector< Region >::const_reverse_iterator rbegin() const { return _regions.rbegin(); }
    std::vector< Region >::const_reverse_iterator rend() const   { return _regions.rend(); }


    size_t size() const                                 { return _regions.size(); }


  protected:

    Regions( const std::vector<Region> &rev );

    std::vector< Region > _regions;

  };


}
