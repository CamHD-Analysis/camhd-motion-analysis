#~ finds curlpp

find_package(CURL REQUIRED)

# set(store_cfls ${CMAKE_FIND_LIBRARY_SUFFIXES})
# set(CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".a" )

set(CURLPP_FIND_NAMES curlpp libcurlpp )
set(CURLPP_INCLUDE_PREFIX "curlpp/")
set(CURLPP_INCLUDE_SEARCHES "cURLpp.hpp")


find_path(CURLPP_INCLUDE_DIR NAMES ${CURLPP_INCLUDE_SEARCHES} PATH_SUFFIXES ${CURLPP_INCLUDE_PREFIX})
find_library(CURLPP_LIBRARY NAMES ${CURLPP_FIND_NAMES} )

set(CURLPP_LIBRARIES ${CURL_LIBRARIES} ${CURLPP_LIBRARY})
set(CURLPP_INCLUDE_DIRS ${CURL_INCLUDE_DIRS} ${CURLPP_INCLUDE_DIR})

include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
find_package_handle_standard_args(CURLpp DEFAULT_MSG CURLPP_LIBRARY CURLPP_INCLUDE_DIR)

mark_as_advanced(CURLPP_LIBRARY CURLPP_INCLUDE_DIR)

message(${CURLPP_LIBRARY})
#
# set(CMAKE_FIND_LIBRARY_SUFFIXES "${store_cfls}")