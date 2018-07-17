cmake_minimum_required(VERSION 3.5)
if( NOT FIPS_IMPORT )
        get_filename_component(FIPS_ROOT_DIR "../fips" ABSOLUTE)
        include("${FIPS_ROOT_DIR}/cmake/fips.cmake")

        fips_setup(PROJECT camhd_motion_tracking)
endif()

## Include Find*.cmake files provided with project.
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

find_package(OpenCV COMPONENTS ${OPENCV_COMPONENTS} REQUIRED)
find_package(Boost COMPONENTS filesystem system REQUIRED)
find_package(Eigen3 REQUIRED )
find_package(CURLpp REQUIRED )
find_package(Ceres REQUIRED )

add_compile_options(-std=c++11)

## Set global include paths
set( CAMHD_MOTRACK_INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/lib
	${EIGEN3_INCLUDE_DIR}
  ${OpenCV_INCLUDE_DIRS}
	${CURLPP_INCLUDE_DIRS}
	${CERES_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/thirdparty/json/src )
include_directories( ${CAMHD_MOTRACK_INCLUDE_DIRS} )

set( COMMON_LIBRARIES
      ${CURLPP_LIBRARY}
      ${OpenCV_LIBRARIES}
      ${Boost_LIBRARIES}
			${CERES_LIBRARIES}
			glog								# Ceres uses glog
			curl )


option(USE_OPENMP "Enable OpenMP suport" OFF )
if( USE_OPENMP )
	add_definitions( -DUSE_OPENMP)
	add_compile_options(-fopenmp)
	list(APPEND COMMON_LIBRARIES gomp )

	if( APPLE )
		set( CC /usr/local/opt/llvm/bin/clang++ )
		list( APPEND CMAKE_CXX_FLAGS LDFLAGS="-L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib")
	endif()

endif()

fips_add_subdirectory("lib/")

if( NOT FIPS_IMPORT )

  fips_add_subdirectory("tools/")

  if( FIPS_UNITTESTS )
    fips_add_subdirectory( test/unit/ )
  endif()

  fips_finish()
endif()
