add_library(zen_syntax STATIC
    src/LanguageRules.cpp
    include/zen/syntax/LanguageRules.h
)
add_library(zen::syntax ALIAS zen_syntax)

target_include_directories(zen_syntax PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(zen_syntax
    PUBLIC
        Qt6::Core
        zen::core
)

zen_target_defaults(zen_syntax)
zen_enable_sanitizers(zen_syntax)
