
#include <gtest/gtest.h>

#include "camhd_client.h"
using namespace CamHDMotionTracking;

#include "test_data.h"

TEST(test_camhd_client, test_synchronous) {
  auto movie( CamHDClient::getMovie( MovieUrl ) );
}
