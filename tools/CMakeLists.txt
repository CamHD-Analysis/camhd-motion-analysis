
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CONAN_INCLUDE_DIRS}
)

link_directories( ${CONAN_LIBRARY_DIR} )

set( TOOLS_LIBRARIES
  camhd
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


# add_executable(motrack motion_track.cpp )
# target_link_libraries( motrack
# ${TOOLS_LIBRARIES}
# )


add_executable(frame_stats frame_stats.cpp )
target_link_libraries( frame_stats
  ${TOOLS_LIBRARIES}
 )
#
#  add_executable(annotate annotate.cpp )
#  target_link_libraries( annotate
#    ${TOOLS_LIBRARIES}
#   )
#
#
# add_executable(stitch stitch.cpp )
# target_link_libraries( stitch
#   ${TOOLS_LIBRARIES}
#  )


 install(TARGETS frame_stats
 	RUNTIME DESTINATION bin
 )
