FIND_LIBRARY(OPENGL_LIBRARY OpenGL)
find_library(COCOA_LIBRARY Cocoa)

# set(CMAKE_CXX_CLANG_TIDY "clang-tidy")

add_compile_options(
    -Wall -Wextra -Werror
    -Wno-deprecated-volatile)

add_subdirectory(shaders)

add_executable(game
    asset_loader.cpp
    entity_editor.cpp
    image.cpp
    imgui.cpp
    input.cpp
    log.cpp
    main.cpp
    physics.cpp
    physics/body.cpp
    physics/collision_type.cpp
    physics/debug_draw.cpp
    physics/plugin.cpp
    physics/space.cpp
    render/line_renderer.cpp
    render/plugin.cpp
    render/quad_renderer.cpp)
set_property(TARGET game PROPERTY CXX_STANDARD 20)
target_link_libraries(game PRIVATE
    shaders
    fmt
    spdlog::spdlog
    sokol
    imgui
    glm
    stb
    EnTT
    chipmunk
    ${OPENGL_LIBRARY}
    ${COCOA_LIBRARY}
)
