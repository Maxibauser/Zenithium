cmake_minimum_required(VERSION 3.24)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(ZenithiumVersion)

project(Zenithium
    VERSION ${ZENITHIUM_VERSION}
    DESCRIPTION "A modern cross-platform code IDE"
    HOMEPAGE_URL "https://github.com/zenithium/zenithium"
    LANGUAGES CXX
)

# ---- Global settings ----
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(ZenithiumCompilerFlags)
include(ZenithiumSanitizers)
include(ZenithiumQtHelpers)

# ---- Options ----
option(ZEN_BUILD_TESTS      "Build the Zenithium test suite"     ON)
option(ZEN_BUILD_TOOLS      "Build developer helper tools"       OFF)
option(ZEN_ENABLE_SANITIZERS "Enable ASan/UBSan on debug builds" OFF)

# ---- Qt6 ----
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Svg)
qt_standard_project_setup()

# ---- Subprojects ----
add_subdirectory(engine)
add_subdirectory(ui)
add_subdirectory(app)

if(ZEN_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ---- Install / package ----
include(ZenithiumPackaging OPTIONAL)

message(STATUS "")
message(STATUS "Zenithium ${PROJECT_VERSION}")
message(STATUS "  Build type : ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler   : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Qt         : ${Qt6_VERSION}")
message(STATUS "  Tests      : ${ZEN_BUILD_TESTS}")
message(STATUS "")
