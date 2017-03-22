#pragma once

#include <string>
#include <stdio.h>
#include <future>

namespace CamHDMotionTracking {

  using std::string;

  struct HTTPResult {

    HTTPResult(  );

    int httpStatus;
    std::string body;

  };

  class HTTPRequest {
  public:

      HTTPRequest( const string &url );

      HTTPResult operator()();
      HTTPResult perform();

      //size_t HeaderCallback(char* ptr, size_t size, size_t nmemb );
      size_t WriteCallback(char* ptr, size_t size, size_t nmemb);

      //std::promise<HTTPResult> promise;
      const HTTPResult result() const { return _result; }

      static HTTPResult Get( const string &url );

    protected:

      HTTPResult _result;
      std::string _url;

  };

}
