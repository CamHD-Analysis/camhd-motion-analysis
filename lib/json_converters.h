#pragma once

#include "json.hpp"
using json = nlohmann::json;

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

namespace boost {
  namespace filesystem {

    // ==== (Un)Serializers for boost::filesystem::path ====
    void to_json(json& j, const fs::path& p) {
        j = p.string();
    }

    void from_json(const json& j, fs::path& p) {
        p = j.get<std::string>();
    }

  }
}

namespace cv {

    // ==== (Un)Serializers for cv::Vec_ ====
    template <typename _Tp>
    void to_json(json& j, const cv::Vec<_Tp, 4>& p) {
         std::array<_Tp,4> a = {p[0],p[1],p[2],p[3]};
         j = a;
    }

    template <typename _Tp>
    void from_json(const json& j, cv::Vec<_Tp,4>& p) {
        std::array<_Tp,4> a = j;
        p( cv::Vec<_Tp,4>(a[0],a[1],a[2],a[3]) );
    }



} // namespace ns
