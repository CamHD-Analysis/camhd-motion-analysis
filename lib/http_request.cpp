

#include "http_request.h"

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>

namespace CamHDMotionTracking {

  using namespace std::placeholders;

  HTTPRequest::HTTPRequest( const string &url )
    : _url(url)
  {
  }

HTTPResult HTTPRequest::operator()( ) {
  return perform();
  // Trigger future
  //promise.set_value(_result);

  //return request.result();
}

  HTTPResult HTTPRequest::perform( ) {
    try {
      curlpp::Easy request;
      request.setOpt(new curlpp::options::Url(_url) );

      curlpp::types::WriteFunctionFunctor functor = std::bind(&HTTPRequest::WriteCallback, this, _1, _2, _3);

      curlpp::options::WriteFunction *test = new curlpp::options::WriteFunction(functor);
      request.setOpt(test);

      request.perform();

      return result();
    }
    catch( cURLpp::RuntimeError &e ) {

    }
    catch( cURLpp::LogicError &e ) {

    }
  }


  size_t HTTPRequest::WriteCallback(char* ptr, size_t size, size_t nmemb) {
    // Drop on floor

    _result.body.append( ptr, size*nmemb );

    return _result.body.size();
  }

  //=================================================
  HTTPResult::HTTPResult( )
    :body()
  {}


  //============ Data accessors =====



}
