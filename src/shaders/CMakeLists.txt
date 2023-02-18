set(GENERATED_HEADERS "")
function(add_shader src)
    add_custom_command(
        PRE_BUILD
        OUTPUT ${src}.h
        COMMAND sokol-shdc --input ${CMAKE_CURRENT_SOURCE_DIR}/${src}
            --output ${src}.h --slang glsl330
        DEPENDS ${src}
    )

    # XXX need list here, and maybe better options to get into global scope
    set(GENERATED_HEADERS "${GENERATED_HEADERS}${src}.h;" PARENT_SCOPE)
endfunction()

# add each shader
add_shader(quad.glsl)
add_shader(line.glsl)

add_library(shaders INTERFACE ${GENERATED_HEADERS})
# shaders are generated in build/src, but allow them to be included using
# double-quotes: `#include "shaders/x.h"`
target_compile_options(shaders INTERFACE
    -iquote ${CMAKE_CURRENT_BINARY_DIR}/..)
