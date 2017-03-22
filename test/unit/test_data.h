#pragma once

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

const fs::path MovieUrl("https://github.com/amarburg/go-lazyfs-testfiles/blob/master/CamHD_Vent_Short.mov?raw=true");


const string TestJson("{\n\"URL\": \"https://rawdata.oceanobservatories.org/files//RS03ASHS/PN03B/06-CAMHDA301/2017/01/01/CAMHDA301-20170101T000500.mov\",\n\"NumFrames\": 10420,\n\"Duration\": 347.68066\n}");
const fs::path TestJsonUrl("https://rawdata.oceanobservatories.org/files//RS03ASHS/PN03B/06-CAMHDA301/2017/01/01/CAMHDA301-20170101T000500.mov");
const float TestJsonDuration = 347.68066;
const int TestJsonNumFrames = 10420;
