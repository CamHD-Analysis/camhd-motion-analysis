
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set( COMMON_LIBRARIES
      camhd_motrack
      ${G3LOG_LIB}
      ${OpenCV_LIBRARIES}
      ${Boost_LIBRARIES} )

# build image node
add_executable(motrack MotionTrack.cpp )
target_link_libraries( motrack
            ${COMMON_LIBRARIES} )

# build image node
add_executable(optical_flow OpticalFlow.cpp )
target_link_libraries( optical_flow
            ${COMMON_LIBRARIES} )

if( USE_GPU )
	add_executable(motrack_gpu MotionTrackGpu.cpp )
	target_link_libraries( motrack_gpu
    camhd_motrack
            ${COMMON_LIBRARIES} )
endif()
