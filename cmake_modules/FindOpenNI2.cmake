###############################################################################
# FindOpenNI2.cmake
#
# This sets the following variables:
# OPENNI2_FOUND - True if OPENNI was found.
# OPENNI2_INCLUDE_DIRS - Directories containing the OPENNI include files.
# OPENNI2_LIBRARIES - Libraries needed to use OPENNI.
# OPENNI2_REDIST_DIR - Run-time libraries that must be copied to the binary directory

### set(OPENNI2_DEFINITIONS ${PC_OPENNI_CFLAGS_OTHER})

find_path(OPENNI2_INCLUDE_DIR OpenNI.h
          PATHS "${OPENNI2_ROOT}/Include"
                /usr/include/openni2
                /usr/include/ni2
                "$ENV{PROGRAMFILES}/OpenNI2/Include"
                "$ENV{PROGRAMW6432}/OpenNI2/Include"
                "$ENV{PROGRAMFILES(x86)}/OpenNI2/Include"
          )

find_library(OPENNI2_LIBRARY
             NAMES OpenNI2
             PATHS  "${OPENNI2_ROOT}/Lib"
                    /usr/lib
                    "$ENV{PROGRAMFILES}/OpenNI2/Redist"
                    "$ENV{PROGRAMW6432}/OpenNI2/Redist"
                    "$ENV{PROGRAMW6432}/OpenNI2"
             PATH_SUFFIXES lib lib64
)

find_path(OPENNI2_REDIST_DIR OpenNI.ini
          PATHS "${OPENNI2_ROOT}/Redist"
                "$ENV{PROGRAMFILES}/OpenNI2/Redist"
                "$ENV{PROGRAMW6432}/OpenNI2/Redist"
                "$ENV{PROGRAMFILES(x86)}/OpenNI2/Redist"
          )

set(OPENNI2_INCLUDE_DIRS ${OPENNI2_INCLUDE_DIR})
set(OPENNI2_LIBRARIES ${OPENNI2_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenNI2 DEFAULT_MSG OPENNI2_LIBRARY OPENNI2_INCLUDE_DIR OPENNI2_REDIST_DIR)

mark_as_advanced(OPENNI2_LIBRARY OPENNI2_INCLUDE_DIR)
if(OPENNI2_FOUND)
  message(STATUS "OpenNI2 found (include: ${OPENNI2_INCLUDE_DIR}, lib: ${OPENNI2_LIBRARY}, redist: ${OPENNI2_REDIST_DIR})")
endif(OPENNI2_FOUND)
