include_directories( ${GTEST_INCLUDE_DIRS}
                     ${CMAKE_SOURCE_DIR}/lib )

file( GLOB UNIT_TEST_SRCS test_*.cpp )

add_executable( camhd_motrack_unit_test
              ${UNIT_TEST_SRCS} )

link_directories( ${GTEST_LIBRARY_DIR} )

target_link_libraries( camhd_motrack_unit_test
          ${CONAN_LIBS}
          camhd_motrack )

# Don't use ctest, instead just execture the Google test binaries
add_custom_target( unit_test
                  COMMAND ./camhd_motrack_unit_test
                  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/bin/
                  DEPENDS camhd_motrack_unit_test )
