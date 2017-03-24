
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CONAN_INCLUDE_DIRS}
)

link_directories( ${CONAN_LIBRARY_DIR} )

set( TOOLS_LIBRARIES
  camhd_motrack
  ${COMMON_LIBRARIES}
  )

add_executable(motrack motion_track.cpp )
target_link_libraries( motrack
${TOOLS_LIBRARIES}
)


add_executable(frame_stats frame_stats.cpp )
target_link_libraries( frame_stats
  ${TOOLS_LIBRARIES}
 )

# if( USE_GPU )
# 	add_executable(motrack_gpu MotionTrackGpu.cpp )
# 	target_link_libraries( motrack_gpu
#     camhd_motrack
#             ${COMMON_LIBRARIES} )
# endif()
