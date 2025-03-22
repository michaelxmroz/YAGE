# Findshaderc.cmake
# Finds the shaderc library
#
# This will define the following variables:
#   SHADERC_FOUND        - True if the system has the shaderc library
#   SHADERC_INCLUDE_DIRS - The shaderc include directories
#   SHADERC_LIBRARIES    - The shaderc libraries for linking
#
# and the following imported targets:
#   shaderc::shaderc - The shaderc library

# Debug output
message(STATUS "Looking for shaderc...")
message(STATUS "  VCPKG_INSTALLED_DIR: ${VCPKG_INSTALLED_DIR}")
message(STATUS "  VCPKG_TARGET_TRIPLET: ${VCPKG_TARGET_TRIPLET}")

# Additional search paths for vcpkg
set(VCPKG_INCLUDE_PATHS
  "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include"
  "${CMAKE_SOURCE_DIR}/vcpkg/installed/${VCPKG_TARGET_TRIPLET}/include"
  "${CMAKE_SOURCE_DIR}/vcpkg/packages/shaderc_${VCPKG_TARGET_TRIPLET}/include"
)

# Include directories
find_path(SHADERC_INCLUDE_DIR
  NAMES shaderc/shaderc.h
  PATHS
    ${VCPKG_INCLUDE_PATHS}
    /usr/include
    /usr/local/include
    /opt/local/include
  DOC "shaderc include directory"
)

# Additional search paths for vcpkg libraries
set(VCPKG_LIB_PATHS
  "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib"
  "${CMAKE_SOURCE_DIR}/vcpkg/installed/${VCPKG_TARGET_TRIPLET}/lib"
  "${CMAKE_SOURCE_DIR}/vcpkg/packages/shaderc_${VCPKG_TARGET_TRIPLET}/lib"
)

# Library
find_library(SHADERC_LIBRARY
  NAMES 
    shaderc
    shaderc_shared
    shaderc_combined
    libshaderc.a
    libshaderc_combined.a
  PATHS
    ${VCPKG_LIB_PATHS}
    /usr/lib
    /usr/local/lib
    /opt/local/lib
  DOC "shaderc library"
)

# Print results for debugging
message(STATUS "  SHADERC_INCLUDE_DIR: ${SHADERC_INCLUDE_DIR}")
message(STATUS "  SHADERC_LIBRARY: ${SHADERC_LIBRARY}")

# Set result variables
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(shaderc
  REQUIRED_VARS
    SHADERC_LIBRARY
    SHADERC_INCLUDE_DIR
)

if(SHADERC_FOUND)
  set(SHADERC_LIBRARIES ${SHADERC_LIBRARY})
  set(SHADERC_INCLUDE_DIRS ${SHADERC_INCLUDE_DIR})
  
  message(STATUS "  shaderc found: ${SHADERC_FOUND}")
  message(STATUS "  SHADERC_LIBRARIES: ${SHADERC_LIBRARIES}")
  message(STATUS "  SHADERC_INCLUDE_DIRS: ${SHADERC_INCLUDE_DIRS}")
  
  if(NOT TARGET shaderc::shaderc)
    add_library(shaderc::shaderc UNKNOWN IMPORTED)
    set_target_properties(shaderc::shaderc PROPERTIES
      IMPORTED_LOCATION "${SHADERC_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${SHADERC_INCLUDE_DIR}"
    )
  endif()
else()
  message(WARNING "shaderc not found. Make sure it's installed or built by vcpkg.")
endif()

mark_as_advanced(SHADERC_INCLUDE_DIR SHADERC_LIBRARY) 