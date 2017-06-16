
#include <sstream>
#include <iostream>


#include "http_request.h"

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Infos.hpp>
#include <curlpp/Exception.hpp>

namespace CamHDMotionTracking {

  using namespace std::placeholders;
  using namespace std;

  HTTPRequest::HTTPRequest( const string &url )
    : _url(url), _result()
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

      //request.setOpt(new curlpp::options::Header(true));

      request.setOpt( new curlpp::options::FollowLocation(true));
      request.setOpt( new curlpp::options::Timeout(300));

      curlpp::types::WriteFunctionFunctor writeFunctor = std::bind(&HTTPRequest::WriteCallback, this, _1, _2, _3);
      request.setOpt(new curlpp::options::WriteFunction(writeFunctor));

      // curlpp::types::WriteFunctionFunctor headerFunctor = std::bind(&HTTPRequest::HeaderCallback, this, _1, _2, _3);
      // request.setOpt( new curlpp::options::HeaderFunction(headerFunctor));

      request.perform();

      // Request info
      _result.httpStatus = curlpp::infos::ResponseCode::get( request );

      //cout << "Set status to " << _result.httpStatus << endl;

      return result();
    }
    catch( cURLpp::RuntimeError &e ) {
      cout << "Runtime error: " << e.what() << endl;
    }
    catch( cURLpp::LogicError &e ) {
      cout << "Logic error: " << e.what() << endl;
    }

    return HTTPResult();
  }


  size_t HTTPRequest::WriteCallback(char* ptr, size_t size, size_t nmemb) {
    size_t prevSize = _result.body.size();
    _result.body.append( ptr, size*nmemb );
    return _result.body.size() - prevSize;
  }

//   size_t HTTPRequest::HeaderCallback(char* ptr, size_t size, size_t nmemb) {
//     string header( ptr, size*nmemb );
//
// cout << "HeaderCallback:  read " << header.size() << " bytes" << endl;
//     // Parse header
//     stringstream stream(header);
//
//     string line;
//     stream >> line;
//     while( !stream.eof() ) {
//       cout << "Header line: " << line << endl;
//
//       stream >> line;
//     }
//
//
//     return header.size();
//   }

HTTPResult HTTPRequest::Get( const string &url )
{
  HTTPRequest request( url );
  return request.perform();
}


  //=================================================
  HTTPResult::HTTPResult( )
    :body(), httpStatus(0)
  {}


  //============ Data accessors =====



}
