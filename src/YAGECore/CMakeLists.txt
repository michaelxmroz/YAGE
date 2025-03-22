file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
    Source/*.cpp
)

add_library(yagecore STATIC ${CORE_SOURCES})

target_include_directories(yagecore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

target_compile_definitions(yagecore PRIVATE 
    YAGE_CORE_BUILD
)

# Platform-specific configurations
if(WIN32)
    target_compile_definitions(yagecore PRIVATE
        _CRT_SECURE_NO_WARNINGS
    )
endif()

set_target_properties(yagecore PROPERTIES FOLDER "YAGE/Core")
