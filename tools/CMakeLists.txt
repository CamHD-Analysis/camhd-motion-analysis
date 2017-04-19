
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CONAN_INCLUDE_DIRS}
)

link_directories( ${CONAN_LIBRARY_DIR} )

set( TOOLS_LIBRARIES
  ${CAMHD_LIBS} 
  ${COMMON_LIBRARIES}
  )

# add_executable(timelapse TimeLapse.cpp )
# target_link_libraries( timelapse
#             ${COMMON_LIBRARIES} )
#
#
# add_executable(sequence Sequence.cpp )
# target_link_libraries( sequence
#             ${COMMON_LIBRARIES} )


add_executable(motrack motion_track.cpp )
target_link_libraries( motrack
${TOOLS_LIBRARIES}
)


add_executable(frame_stats frame_stats.cpp )
target_link_libraries( frame_stats
  ${TOOLS_LIBRARIES}
 )
