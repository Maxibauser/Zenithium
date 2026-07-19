qt_add_executable(zenithium WIN32
    main.cpp
    Bootstrap.h
    Bootstrap.cpp
)

set_target_properties(zenithium PROPERTIES
    OUTPUT_NAME "Zenithium"
)

target_link_libraries(zenithium PRIVATE
    Qt6::Widgets
    zen::ui
    zen::core
)

zen_target_defaults(zenithium)
zen_enable_sanitizers(zenithium)
zen_windeployqt(zenithium)
