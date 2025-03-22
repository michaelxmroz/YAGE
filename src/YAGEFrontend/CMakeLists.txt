file(GLOB_RECURSE FRONTEND_SOURCES CONFIGURE_DEPENDS
    *.cpp
)

# Filter out platform-specific files based on OS
if(WIN32)
    list(FILTER FRONTEND_SOURCES EXCLUDE REGEX ".*Linux\\.cpp$")
    list(FILTER FRONTEND_SOURCES EXCLUDE REGEX ".*imgui_impl_x11\\.cpp$")
elseif(UNIX)
    list(FILTER FRONTEND_SOURCES EXCLUDE REGEX ".*Win32\\.cpp$")
endif()

# Find required packages
find_package(PortAudio REQUIRED)
find_package(shaderc REQUIRED)
if(UNIX)
    find_package(X11 REQUIRED)
    find_package(Threads REQUIRED)
endif()

add_executable(yagefrontend ${FRONTEND_SOURCES})

target_compile_definitions(yagefrontend PRIVATE 
    VK_NO_PROTOTYPES VK_USE_PLATFORM_XLIB_KHR 
)

# Path to the shader source file
set(SHADER_SRC "${CMAKE_CURRENT_SOURCE_DIR}/main.glsl")

# Destination path for the copied file
set(SHADER_DST "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/main.glsl")

# Custom command to copy the file
add_custom_command(
    OUTPUT ${SHADER_DST}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SHADER_SRC} ${SHADER_DST}
    DEPENDS ${SHADER_SRC}
    COMMENT "Copying shader to output directory"
)

# Custom target to trigger the command during build
add_custom_target(copy_shader ALL DEPENDS ${SHADER_DST})

# Make sure the shader copy happens before running the executable
add_dependencies(yagefrontend copy_shader)

# Include frontend, external libs, and core API headers
target_include_directories(yagefrontend
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/externalLibs/imgui
        ${PROJECT_SOURCE_DIR}/externalLibs/volk
        ${PROJECT_SOURCE_DIR}/externalLibs/vulkan/include
        ${PROJECT_SOURCE_DIR}/src/YAGECore/include
        ${PORTAUDIO_INCLUDE_DIRS}
)

# Add platform-specific include directories
if(UNIX)
    target_include_directories(yagefrontend PRIVATE
        ${X11_INCLUDE_DIR}
    )
endif()

# Link against yagecore
target_link_libraries(yagefrontend
    PRIVATE
        yagecore
        ${PORTAUDIO_LIBRARIES}
        shaderc::shaderc
)

# Add platform-specific libraries
if(UNIX)
    target_link_libraries(yagefrontend PRIVATE
        ${X11_LIBRARIES}
        Threads::Threads
        -ldl
    )
endif()

# Add all ImGui sources directly to the build
file(GLOB_RECURSE IMGUI_SOURCES 
    ${PROJECT_SOURCE_DIR}/externalLibs/imgui/*.cpp
)

# Filter out platform-specific ImGui backends
if(WIN32)
    list(FILTER IMGUI_SOURCES EXCLUDE REGEX ".*_glfw\\.cpp$")
    list(FILTER IMGUI_SOURCES EXCLUDE REGEX ".*_x11\\.cpp$")
elseif(UNIX)
    list(FILTER IMGUI_SOURCES EXCLUDE REGEX ".*_win32\\.cpp$")
endif()

target_sources(yagefrontend PRIVATE ${IMGUI_SOURCES})

set_target_properties(yagefrontend PROPERTIES FOLDER "YAGE/Frontend")