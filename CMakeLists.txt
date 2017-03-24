cmake_minimum_required(VERSION 2.8.7)
project(camhd_motion_tracking)

set(CMAKE_PREFIX_PATH /usr)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
list(APPEND OPENCV_COMPONENTS core highgui imgproc video calib3d  )

option(USE_GPU "Build GPU-accelerated tools" OFF)
if( USE_GPU )
	add_definitions( -DUSE_GPU )
	list(APPEND OPENCV_COMPONENTS gpu )
endif()

find_package( CUDA )
if( CUDA_VERSION )
	message("Found CUDA version ${CUDA_VERSION}")
	if(${CUDA_VERSION} VERSION_EQUAL "8.0")
		message("Disabling CUDA_USE_STATIC_CUDA_RUNTIME")
		set( CUDA_USE_STATIC_CUDA_RUNTIME OFF CACHE BOOL "" FORCE  )
	endif()
  find_package( CUDA )
endif()

find_package(OpenCV 2.4 COMPONENTS ${OPENCV_COMPONENTS} REQUIRED)
find_package(Boost COMPONENTS filesystem system REQUIRED)
find_package(Eigen3 REQUIRED )
find_package(CURLpp REQUIRED )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

## Set global include paths
set( CAMHD_MOTRACK_INCLUDE_DIRS
	/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/
	${CONAN_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/include
	${EIGEN3_INCLUDE_DIR}
  ${OpenCV_INCLUDE_DIRS}
	${CURLPP_INCLUDE_DIRS}
${CMAKE_SOURCE_DIR}/thirdparty/json/src )
include_directories( ${CAMHD_MOTRACK_INCLUDE_DIRS} )

set( COMMON_LIBRARIES
      ${CURLPP_LIBRARY}
      ${CONAN_LIBS}
      ${OpenCV_LIBRARIES}
      ${Boost_LIBRARIES}
		curl )

add_subdirectory("lib/")
add_subdirectory("tools/")

option( BUILD_UNIT_TESTS "Build unit tests" OFF )
if( BUILD_UNIT_TESTS )

  add_subdirectory( test/unit/ )

endif()
