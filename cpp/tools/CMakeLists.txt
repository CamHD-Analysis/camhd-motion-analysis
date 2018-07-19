
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CONAN_INCLUDE_DIRS}
)

link_directories( ${CONAN_LIBRARY_DIR} )

set( TOOLS_LIBRARIES
  camhd
  ${COMMON_LIBRARIES}
  )


fips_begin_app( frame_stats cmdline )

  fips_files( frame_stats.cpp )
  fips_deps( camhd )

fips_end_app()


 install(TARGETS frame_stats
 	RUNTIME DESTINATION bin
 )
