# Deploys Qt runtime alongside the built executable on Windows so the app can
# be launched from the build directory without a system-wide Qt install.
function(zen_windeployqt tgt)
    if(NOT WIN32)
        return()
    endif()
    find_program(WINDEPLOYQT_EXE windeployqt HINTS "${Qt6_DIR}/../../../bin")
    if(NOT WINDEPLOYQT_EXE)
        message(WARNING "windeployqt not found; ${tgt} may fail to launch from build dir")
        return()
    endif()
    add_custom_command(TARGET ${tgt} POST_BUILD
        COMMAND "${WINDEPLOYQT_EXE}"
                --no-translations --no-system-d3d-compiler --no-opengl-sw
                --no-quick-import
                "$<TARGET_FILE:${tgt}>"
        COMMENT "Deploying Qt runtime for ${tgt}"
        VERBATIM
    )
endfunction()
