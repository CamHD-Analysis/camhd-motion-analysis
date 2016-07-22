# Set source files
set(camhd_motrack_SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/foo.cpp
)


include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# build shared library.
add_library(camhd_motrack SHARED ${camhd_motrack_SOURCE_FILES})
# target_link_libraries(camhd_motrack
#   ${Boost_LIBRARIES}
# )
