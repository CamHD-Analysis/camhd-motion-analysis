#pragma once

#include <chrono>

struct Timer {
  Timer()
  : _start(std::chrono::system_clock::now()), _stop(_start), _running(true)
  {;}

  void start()
  {
    _start = std::chrono::system_clock::now();
    _running = true;
  }

  void stop()
  {
    _stop = std::chrono::system_clock::now();
    _running = false;
  }

  double seconds( void )
  {
    if( _running ) stop();
    std::chrono::duration<double> elapsedSeconds = _stop-_start;
    return elapsedSeconds.count();
  }

  std::chrono::time_point<std::chrono::system_clock> _start, _stop;
  bool _running;

};
