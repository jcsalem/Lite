cmake_minimum_required(VERSION 2.6)
project(Lite)

#-------------------------------------------------
# CMAKE customizations
#-------------------------------------------------
# Default debug
if(CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE Debug)
endif()

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake_modules")

#-------------------------------------------------
# SFML
#-------------------------------------------------
OPTION(NO_SFML "NO_SFML" OFF)
IF(NO_SFML)
   message(STATUS "SFML Windowing: Not installed because NO_SFML was ${NO_SFML}")
ELSE(NO_SFML)
   IF(WIN32)
      SET(SFML_STATIC_LIBRARIES TRUE)
      SET(SFML_ROOT "${PROJECT_SOURCE_DIR}/../SFML-2.1")
   ENDIF(WIN32)

   IF(NOT MSVC)
      # SFML too complicated to support for MSVS
      find_package(SFML 2 COMPONENTS graphics window system)
      if (SFML_FOUND)
        add_definitions(-DHAS_SFML=1)
    	include_directories( ${SFML_INCLUDE_DIR})
      ELSE(SFML_FOUND)
        message(WARNING "SFML Windowing: Not installed because the library wasn't found.")
      ENDIF(SFML_FOUND)
   ELSE(NOT MSVC)
      message(STATUS "SFML Windowing: Not supported for Visual Studio.")
   ENDIF(NOT MSVC)
ENDIF(NO_SFML)

#-------------------------------------------------
# GLOBAL COMPILE/LINK OPTIONS
#-------------------------------------------------
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
# LIBRARY is for dynamic libraries; ARCHIVE is for static libraries
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
include_directories("${PROJECT_SOURCE_DIR}/libsrc" "${PROJECT_SOURCE_DIR}/src")
link_directories("${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

#-------------------------------------------------
# Subdirectories
#-------------------------------------------------
add_subdirectory(libsrc)
add_subdirectory(src)

