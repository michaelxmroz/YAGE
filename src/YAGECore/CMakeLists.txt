file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
    Source/*.cpp
)

add_library(yagecore STATIC ${CORE_SOURCES})

target_include_directories(yagecore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

target_compile_definitions(yagecore PRIVATE YAGE_CORE_BUILD)

set_target_properties(yagecore PROPERTIES FOLDER "YAGE/Core")
