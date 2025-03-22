cmake_minimum_required(VERSION 3.15)
project(YAGE LANGUAGES CXX)

# Print helpful information
message(STATUS "Configuring YAGE build...")
message(STATUS "CMake version: ${CMAKE_VERSION}")
message(STATUS "System: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Processor: ${CMAKE_SYSTEM_PROCESSOR}")

# Add our cmake modules to module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Set compiler based on platform
if(UNIX)
    # Check if clang is available
    find_program(CLANG_EXECUTABLE clang)
    find_program(CLANGPP_EXECUTABLE clang++)
    
    if(CLANG_EXECUTABLE AND CLANGPP_EXECUTABLE)
        set(CMAKE_C_COMPILER ${CLANG_EXECUTABLE})
        set(CMAKE_CXX_COMPILER ${CLANGPP_EXECUTABLE})
        message(STATUS "Using Clang compiler: ${CLANG_EXECUTABLE}, ${CLANGPP_EXECUTABLE}")
    else()
        message(WARNING "Clang compiler not found, using default system compiler")
    endif()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define platform-specific macros
if(WIN32)
    add_definitions(-DYAGE_PLATFORM_WINDOWS=1)
    add_definitions(-DYAGE_PLATFORM_LINUX=0)
    add_definitions(-DYAGE_PLATFORM_UNIX=0)
elseif(UNIX)
    add_definitions(-DYAGE_PLATFORM_WINDOWS=0)
    add_definitions(-DYAGE_PLATFORM_UNIX=1)
    if(APPLE)
        add_definitions(-DYAGE_PLATFORM_LINUX=0)
    else()
        add_definitions(-DYAGE_PLATFORM_LINUX=1)
    endif()
endif()

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
    set(CMAKE_BUILD_TYPE "Release")
endif()

# Set final output directory: bin/<arch>/<build type>
set(FINAL_BIN_DIR ${CMAKE_SOURCE_DIR}/bin/${ARCH_DIR}/${CMAKE_BUILD_TYPE})

# Set all output directories globally
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${FINAL_BIN_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${FINAL_BIN_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${FINAL_BIN_DIR})

# Ensure vcpkg integration
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

# Enable folders in IDEs (e.g., Visual Studio)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Display some build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "Target architecture: ${ARCH_DIR}")
message(STATUS "Output directory: ${FINAL_BIN_DIR}")

# External dependencies
add_subdirectory(src/YAGECore)
add_subdirectory(src/YAGEFrontend)
