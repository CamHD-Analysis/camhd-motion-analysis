#pragma once

#include <string>

#include <curlpp/cURLpp.hpp>

#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>

std::shared_ptr<curlpp::Cleanup> commonInit( const std::string &name  ) {
  auto worker = g3::LogWorker::createLogWorker();
  auto handle= worker->addDefaultLogger(name,".");
  g3::initializeLogging(worker.get());

  // RAAI initializer for curlpp
  return std::shared_ptr<curlpp::Cleanup>(new curlpp::Cleanup);
}
