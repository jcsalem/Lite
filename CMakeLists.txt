cmake_minimum_required(VERSION 2.6)
project(Lite)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

include_directories("${PROJECT_SOURCE_DIR}/libsrc" "${PROJECT_SOURCE_DIR}/src")
link_directories("${CMAKE_BINARY_DIR}/lib")

# Some examples explicitly set the path to ${PROJECT_SOURCE_DIR}
# This may be needed when you want to build out of the source dir rather than the bin directory
add_subdirectory(libsrc)
add_subdirectory(src)
