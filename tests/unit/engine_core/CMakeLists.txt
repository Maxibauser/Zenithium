find_package(Qt6 REQUIRED COMPONENTS Test)

qt_add_executable(test_engine_core VersionTest.cpp)
target_link_libraries(test_engine_core PRIVATE zen::core Qt6::Test)
zen_target_defaults(test_engine_core)

add_test(NAME engine_core.version COMMAND test_engine_core)

if(WIN32)
    get_target_property(_qt_core_loc Qt6::Core LOCATION)
    get_filename_component(_qt_bin_dir "${_qt_core_loc}" DIRECTORY)
    set_tests_properties(engine_core.version PROPERTIES
        ENVIRONMENT "PATH=${_qt_bin_dir};$ENV{PATH}"
    )
endif()
