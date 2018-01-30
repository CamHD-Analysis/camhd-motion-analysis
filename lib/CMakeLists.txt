set( libcamhd_SRCS
      camhd_client.cpp
      camhd_movie.cpp
      http_request.cpp
      regions.cpp
      json_contents.cpp
      similarity.cpp
    movie_workers/optical_flow.cpp )

if( USE_GPU )
  list(APPEND libcamhd_SRCS
  movie_workers/gpu_optical_flow.cpp )
endif()

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

fips_begin_module( camhd )

  fips_files( ${libcamhd_SRCS} )
  fips_libs( ${COMMON_LIBRARIES} )
  #fips_deps( logger g3log )

fips_end_module()

install(TARGETS camhd
 RUNTIME DESTINATION lib
  ARCHIVE DESTINATION lib
)
