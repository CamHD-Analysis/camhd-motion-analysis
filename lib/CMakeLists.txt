# Set source files
file(GLOB camhd_motrack_SOURCE_FILES *.cpp )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# This needs to be set before add_library
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )

# build shared library.
add_library(camhd_motrack SHARED
  ${camhd_motrack_SOURCE_FILES})

target_link_libraries( camhd_motrack
  ${COMMON_LIBRARIES}
)

# set_target_properties(camhd_motrack PROPERTIES
#         RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BUILD_DIR}/bin
# )
