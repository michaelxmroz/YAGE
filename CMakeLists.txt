cmake_minimum_required(VERSION 3.15)
project(YAGE LANGUAGES CXX)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Detect architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCH_DIR "x64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(ARCH_DIR "x86")
else()
    set(ARCH_DIR "unknown")
endif()

# Default build type fallback (optional)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Unknown")
endif()

# Set final output directory: bin/<arch>/<build type>
set(FINAL_BIN_DIR ${CMAKE_SOURCE_DIR}/bin/${ARCH_DIR}/${CMAKE_BUILD_TYPE})

# Set all output directories globally
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${FINAL_BIN_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${FINAL_BIN_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${FINAL_BIN_DIR})


# Enable folders in IDEs (e.g., Visual Studio)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# External dependencies
add_subdirectory(src/YAGECore)
add_subdirectory(src/YAGEFrontend)
