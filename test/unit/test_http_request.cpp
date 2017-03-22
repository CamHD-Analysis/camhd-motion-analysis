
#include <gtest/gtest.h>

#include "http_request.h"

using namespace CamHDMotionTracking;

const string AlphabetUrl = "https://raw.githubusercontent.com/amarburg/go-lazyfs-testfiles/master/alphabet.fs";

TEST(test_http_request, test_synchronous) {
  HTTPRequest request(AlphabetUrl);
  request.perform();
  //std::future<HTTPResult> future = req.promise.get_future();

  HTTPResult result( request.result() );
}


TEST(test_http_request, test_asynchronous) {
  //std::promise<HTTPResult> promise;
  HTTPRequest request(AlphabetUrl );

  //std::future<HTTPResult> future = promise.get_future();
  auto future = std::async(request);

  future.wait();

  HTTPResult result( future.get() );

  //http.join();
}
