add_library(zen_core STATIC
    src/Version.cpp
    include/zen/core/Version.h
)
add_library(zen::core ALIAS zen_core)

target_include_directories(zen_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_definitions(zen_core PRIVATE
    ZEN_VERSION_MAJOR=${ZENITHIUM_VERSION_MAJOR}
    ZEN_VERSION_MINOR=${ZENITHIUM_VERSION_MINOR}
    ZEN_VERSION_PATCH=${ZENITHIUM_VERSION_PATCH}
    ZEN_VERSION_STRING="${ZENITHIUM_VERSION}"
)

zen_target_defaults(zen_core)
zen_enable_sanitizers(zen_core)
