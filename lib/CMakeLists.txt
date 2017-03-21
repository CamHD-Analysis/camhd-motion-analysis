# Set source files
file(GLOB camhd_motrack_SOURCE_FILES *.cpp )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# build shared library.
add_library(camhd_motrack SHARED
  ${camhd_motrack_SOURCE_FILES})
