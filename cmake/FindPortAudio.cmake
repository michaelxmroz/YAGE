# FindPortAudio.cmake
# Finds the PortAudio library
#
# This will define the following variables:
#   PORTAUDIO_FOUND        - True if the system has the PortAudio library
#   PORTAUDIO_INCLUDE_DIRS - The PortAudio include directories
#   PORTAUDIO_LIBRARIES    - The PortAudio libraries for linking
#
# and the following imported targets:
#   PortAudio::PortAudio - The PortAudio library

# Try to find the package using pkg-config first
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_PORTAUDIO QUIET portaudio-2.0)
endif()

# Debug output
message(STATUS "Looking for PortAudio...")
message(STATUS "  VCPKG_INSTALLED_DIR: ${VCPKG_INSTALLED_DIR}")
message(STATUS "  VCPKG_TARGET_TRIPLET: ${VCPKG_TARGET_TRIPLET}")

# Additional search paths for vcpkg
set(VCPKG_INCLUDE_PATHS
  "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include"
  "${CMAKE_SOURCE_DIR}/vcpkg/installed/${VCPKG_TARGET_TRIPLET}/include"
  "${CMAKE_SOURCE_DIR}/vcpkg/packages/portaudio_${VCPKG_TARGET_TRIPLET}/include"
)

# Include directories
find_path(PORTAUDIO_INCLUDE_DIR
  NAMES portaudio.h
  PATHS
    ${PC_PORTAUDIO_INCLUDE_DIRS}
    ${VCPKG_INCLUDE_PATHS}
    /usr/include
    /usr/local/include
    /opt/local/include
  DOC "PortAudio include directory"
)

# Additional search paths for vcpkg libraries
set(VCPKG_LIB_PATHS
  "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib"
  "${CMAKE_SOURCE_DIR}/vcpkg/installed/${VCPKG_TARGET_TRIPLET}/lib"
  "${CMAKE_SOURCE_DIR}/vcpkg/packages/portaudio_${VCPKG_TARGET_TRIPLET}/lib"
)

# Library
find_library(PORTAUDIO_LIBRARY
  NAMES 
    portaudio
    portaudio_x64
    portaudio_static
    libportaudio.a
  PATHS
    ${PC_PORTAUDIO_LIBRARY_DIRS}
    ${VCPKG_LIB_PATHS}
    /usr/lib
    /usr/local/lib
    /opt/local/lib
  DOC "PortAudio library"
)

# Print results for debugging
message(STATUS "  PORTAUDIO_INCLUDE_DIR: ${PORTAUDIO_INCLUDE_DIR}")
message(STATUS "  PORTAUDIO_LIBRARY: ${PORTAUDIO_LIBRARY}")

# Set result variables
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PortAudio
  REQUIRED_VARS
    PORTAUDIO_LIBRARY
    PORTAUDIO_INCLUDE_DIR
  VERSION_VAR PC_PORTAUDIO_VERSION
)

if(PORTAUDIO_FOUND)
  set(PORTAUDIO_LIBRARIES ${PORTAUDIO_LIBRARY})
  set(PORTAUDIO_INCLUDE_DIRS ${PORTAUDIO_INCLUDE_DIR})
  
  message(STATUS "  PortAudio found: ${PORTAUDIO_FOUND}")
  message(STATUS "  PORTAUDIO_LIBRARIES: ${PORTAUDIO_LIBRARIES}")
  message(STATUS "  PORTAUDIO_INCLUDE_DIRS: ${PORTAUDIO_INCLUDE_DIRS}")
  
  if(NOT TARGET PortAudio::PortAudio)
    add_library(PortAudio::PortAudio UNKNOWN IMPORTED)
    set_target_properties(PortAudio::PortAudio PROPERTIES
      IMPORTED_LOCATION "${PORTAUDIO_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${PORTAUDIO_INCLUDE_DIR}"
    )
  endif()
else()
  message(WARNING "PortAudio not found. Make sure it's installed or built by vcpkg.")
endif()

mark_as_advanced(PORTAUDIO_INCLUDE_DIR PORTAUDIO_LIBRARY) 