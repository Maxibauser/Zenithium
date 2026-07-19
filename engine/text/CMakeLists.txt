add_library(zen_text STATIC
    src/TextBuffer.cpp
    include/zen/text/TextBuffer.h
)
add_library(zen::text ALIAS zen_text)

target_include_directories(zen_text PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(zen_text PUBLIC zen::core)

zen_target_defaults(zen_text)
zen_enable_sanitizers(zen_text)
