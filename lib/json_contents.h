#pragma once

#include <string>

#include "json.hpp"
using json = nlohmann::json;

namespace CamHDMotionTracking {

  static const std::string CONTENTS_JSON_TAG = "contents";

  inline void addJSONContents( json& j, const std::string &name, const std::string &version ) {

    // auto contentsItr = j.find( CONTENTS_JSON_TAG );
    // if( contentsItr == j.end() ) {
    //   j[CONTENTS_JSON_TAG] = json::object();
    // }

    j[name] = version;

  }

}
