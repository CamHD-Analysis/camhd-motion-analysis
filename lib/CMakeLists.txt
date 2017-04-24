set( libcamhd_SRCS
      camhd_client.cpp
      camhd_movie.cpp
      http_request.cpp
    movie_workers/optical_flow.cpp )

if( USE_GPU )
  list(APPEND libcamhd_SRCS
  movie_workers/gpu_optical_flow.cpp )
endif()

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# This needs to be set before add_library .. only on OSX?
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )

# build shared library.
add_library( camhd SHARED ${libcamhd_SRCS} )

target_link_libraries( camhd ${COMMON_LIBRARIES} )

set( CAMHD_LIBS camhd CACHE INTERNAL "" )

# target_link_libraries( camhd
#   ${COMMON_LIBRARIES}
# )
