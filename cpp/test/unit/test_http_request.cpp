
#include <gtest/gtest.h>

#include "http_request.h"

using namespace CamHDMotionTracking;

const string AlphabetUrl = "https://raw.githubusercontent.com/amarburg/go-lazyfs-testfiles/master/alphabet.fs";

static void alphabetTests( const HTTPResult &result )
{
  ASSERT_EQ( result.httpStatus, 200 );
  ASSERT_EQ( result.body.size(), 26 );

  ASSERT_STREQ( result.body.c_str(), "ABCDEFGHIJKLMNOPQRSTUVWXYZ" );
}

TEST(test_http_request, test_synchronous) {
  HTTPRequest request(AlphabetUrl);
  request.perform();
  //std::future<HTTPResult> future = req.promise.get_future();

  HTTPResult result( request.result() );

  alphabetTests( result );
}


TEST(test_http_request, test_asynchronous) {
  HTTPRequest request(AlphabetUrl );
  auto future = std::async(request);

  future.wait();

  HTTPResult result( future.get() );

  alphabetTests( result );
}
