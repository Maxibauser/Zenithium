# Zenithium install + packaging rules.
#
# Layout after `cmake --install <build> --prefix dist/`:
#     dist/
#       Zenithium.exe
#       Qt6*.dll, MSVC runtime, plugins/ ...
#       LICENSE
#       README.md
#
# The Qt DLLs and plugins are staged via `windeployqt` at install time so we
# never ship debug (*d.dll) binaries or half-populated build directories.

include(GNUInstallDirs)

# Everything lands directly under the prefix — Inno Setup expects a flat
# {app}\ directory tree.
install(TARGETS zenithium
    RUNTIME DESTINATION .
    COMPONENT Runtime
)

install(FILES
    "${CMAKE_SOURCE_DIR}/LICENSE"
    "${CMAKE_SOURCE_DIR}/README.md"
    DESTINATION .
    COMPONENT Runtime
)

# ---- Windows: run windeployqt against the installed exe --------------------
if(WIN32)
    find_program(WINDEPLOYQT_EXE windeployqt
        HINTS "${Qt6_DIR}/../../../bin"
    )
    if(WINDEPLOYQT_EXE)
        # We defer the call to install-time so it runs against the FINAL exe
        # in the install tree (release DLLs, no *.pdb/*.ilk clutter).
        install(CODE "
            message(STATUS \"Running windeployqt on \${CMAKE_INSTALL_PREFIX}/Zenithium.exe\")
            execute_process(
                COMMAND \"${WINDEPLOYQT_EXE}\"
                        --release
                        --no-translations
                        --no-system-d3d-compiler
                        --no-opengl-sw
                        --no-quick-import
                        --no-compiler-runtime
                        --dir \"\${CMAKE_INSTALL_PREFIX}\"
                        \"\${CMAKE_INSTALL_PREFIX}/Zenithium.exe\"
                RESULT_VARIABLE _wdq_rc
            )
            if(NOT _wdq_rc EQUAL 0)
                message(FATAL_ERROR \"windeployqt failed with code \${_wdq_rc}\")
            endif()
        " COMPONENT Runtime)
    else()
        message(WARNING "windeployqt not found — install tree will be missing Qt DLLs")
    endif()
endif()

# ---- CPack (portable ZIP) --------------------------------------------------
set(CPACK_PACKAGE_NAME              "Zenithium")
set(CPACK_PACKAGE_VENDOR            "Zenithium")
set(CPACK_PACKAGE_VERSION           "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_HOMEPAGE_URL      "${PROJECT_HOMEPAGE_URL}")
set(CPACK_RESOURCE_FILE_LICENSE     "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Zenithium")

if(WIN32)
    set(CPACK_GENERATOR "ZIP")
    set(CPACK_PACKAGE_FILE_NAME
        "Zenithium-${PROJECT_VERSION}-win64")
endif()

include(CPack)
