cmake_minimum_required(VERSION 2.8.7)
project(camhd_motion_tracking)
include(ExternalProject)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set(RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

list(APPEND OPENCV_COMPONENTS core highgui imgproc video calib3d )

option(USE_GPU "Build GPU-accelerated tools" OFF)
if( USE_GPU )
	add_definitions( -DUSE_GPU )
	list(APPEND OPENCV_COMPONENTS gpu )
endif()

find_package(OpenCV 2.4 COMPONENTS ${OPENCV_COMPONENTS} REQUIRED)
find_package(Boost COMPONENTS filesystem system REQUIRED)
find_package(TCLAP REQUIRED )
find_package(Eigen3 REQUIRED )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include( ${PROJECT_SOURCE_DIR}/cmake/BuildLibVideoTools.cmake )


## Set global include paths
set( CAMHD_MOTRACK_INCLUDE_DIRS
	${G3LOG_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/include
	${EIGEN3_INCLUDE_DIR}
  ${OpenCV_INCLUDE_DIRS} )

include_directories( ${CAMHD_MOTRACK_INCLUDE_DIRS} )

add_subdirectory("lib/")
add_subdirectory("tools/")

option( BUILD_UNIT_TESTS "Build unit tests" ON )
if( BUILD_UNIT_TESTS )
  ## Build a local copy of gtest (as per the GTest FAQ)
  include( ${CMAKE_SOURCE_DIR}/cmake/BuildGTestLibraries.cmake )
	list( APPEND EXTERNAL_PROJECTS gtest )

  add_subdirectory( test/unit/ )

endif()

add_custom_target( deps DEPENDS ${EXTERNAL_PROJECTS} )
