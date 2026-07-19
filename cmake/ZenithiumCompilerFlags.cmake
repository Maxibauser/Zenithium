# Common compiler warning flags applied to Zenithium targets via zen_target_defaults()
function(zen_target_defaults tgt)
    target_compile_features(${tgt} PUBLIC cxx_std_20)

    if(MSVC)
        target_compile_options(${tgt} PRIVATE
            /W4 /permissive- /Zc:__cplusplus /Zc:preprocessor
            /utf-8 /EHsc /MP
        )
        target_compile_definitions(${tgt} PRIVATE
            NOMINMAX
            WIN32_LEAN_AND_MEAN
            _CRT_SECURE_NO_WARNINGS
            UNICODE _UNICODE
        )
    else()
        target_compile_options(${tgt} PRIVATE
            -Wall -Wextra -Wpedantic
            -Wshadow -Wnon-virtual-dtor -Wold-style-cast
            -Wcast-align -Wunused -Woverloaded-virtual
            -Wconversion -Wsign-conversion -Wnull-dereference
            -Wdouble-promotion -Wformat=2
        )
    endif()
endfunction()
