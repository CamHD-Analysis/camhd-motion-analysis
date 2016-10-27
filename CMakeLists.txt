cmake_minimum_required(VERSION 2.8.7)
project(camhd_motion_tracking)
#include(ExternalProject)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set(RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

## Use ccache if available
# find_program(CCACHE_FOUND ccache)
# if(CCACHE_FOUND)
# 		message( "Using ccache" )
#     set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
#     set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
# endif(CCACHE_FOUND)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

list(APPEND OPENCV_COMPONENTS core highgui imgproc video)

option(USE_GPU "Build GPU-accelerated tools" OFF)
if( USE_GPU )
	add_definitions( -DUSE_GPU )
	list(APPEND OPENCV_COMPONENTS gpu )
endif()

find_package(OpenCV 2.4 COMPONENTS ${OPENCV_COMPONENTS} REQUIRED)
find_package(Boost COMPONENTS filesystem system REQUIRED)
find_package(TCLAP REQUIRED )

## C++ flags applied to all builds
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


## Set global include paths
set( CAMHD_MOTRACK_INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/lib
  ${OpenCV_INCLUDE_DIRS} )
include_directories( ${CAMHD_MOTRACK_INCLUDE_DIRS} )

add_subdirectory("lib/")

add_subdirectory("tools/")

option( BUILD_UNIT_TESTS "Build unit tests" ON )
if( BUILD_UNIT_TESTS )
  ## Build a local copy of gtest (as per the GTest FAQ)
  include( ${CMAKE_SOURCE_DIR}/cmake/BuildGTestLibraries.cmake )

  add_subdirectory( test/unit/ )

endif()
