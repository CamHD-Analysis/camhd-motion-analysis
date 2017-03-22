
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CONAN_INCLUDE_DIRS}
)

link_directories( ${CONAN_LIBRARY_DIR} )

set( TOOLS_LIBRARIES
  camhd_motrack
  ${COMMON_LIBRARIES}
  )

add_executable(motrack MotionTrack.cpp )
target_link_libraries( motrack
${TOOLS_LIBRARIES}
)


add_executable(optical_flow OpticalFlow.cpp )
target_link_libraries( optical_flow
  ${TOOLS_LIBRARIES}
 )

# if( USE_GPU )
# 	add_executable(motrack_gpu MotionTrackGpu.cpp )
# 	target_link_libraries( motrack_gpu
#     camhd_motrack
#             ${COMMON_LIBRARIES} )
# endif()
