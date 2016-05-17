
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# build image node
add_executable(camhd_motrack MotionTrack.cpp )
target_link_libraries( camhd_motrack
						${OpenCV_LIBRARIES}
		  			${Boost_LIBRARIES} )
