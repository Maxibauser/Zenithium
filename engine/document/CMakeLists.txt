add_library(zen_document STATIC
    src/Document.cpp
    src/LineDiff.cpp
    include/zen/document/Document.h
    include/zen/document/LineDiff.h
)
add_library(zen::document ALIAS zen_document)

target_include_directories(zen_document PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(zen_document
    PUBLIC
        Qt6::Core
        zen::core
        zen::text
)

zen_target_defaults(zen_document)
zen_enable_sanitizers(zen_document)
