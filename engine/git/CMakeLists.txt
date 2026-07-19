add_library(zen_git STATIC
    src/GitService.cpp
    include/zen/git/GitService.h
)
add_library(zen::git ALIAS zen_git)

target_include_directories(zen_git PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(zen_git
    PUBLIC
        Qt6::Core
        zen::core
)

zen_target_defaults(zen_git)
zen_enable_sanitizers(zen_git)
