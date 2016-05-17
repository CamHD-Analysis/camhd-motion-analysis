
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# build image node
add_executable(motrack MotionTrack.cpp )
target_link_libraries( motrack
						${OpenCV_LIBRARIES}
		  			${Boost_LIBRARIES} )

add_executable(motrack_gpu MotionTrackGpu.cpp )
target_link_libraries( motrack_gpu
						${OpenCV_LIBRARIES}
		  			${Boost_LIBRARIES} )
