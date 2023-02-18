FetchContent_Declare(
    sokol
    GIT_REPOSITORY https://github.com/floooh/sokol
    GIT_TAG 21c9d47c85324104b0a5eeae3e017bd3551df7bb # 2022-12-16
    CONFIGURE_COMMAND ""
    BUILD_COMMAND "")

FetchContent_GetProperties(sokol)
if(NOT sokol_POPULATED)
    FetchContent_Populate(sokol)
endif()

# needed for sokol-imgui bits
FetchContent_GetProperties(imgui)

add_compile_definitions(SOKOL_GLCORE33)
configure_file(sokol_app.m.in ${sokol_SOURCE_DIR}/sokol_app.m)
configure_file(sokol.cpp.in ${sokol_SOURCE_DIR}/sokol.cpp)
add_library(sokol STATIC
    ${sokol_SOURCE_DIR}/sokol.cpp
    ${sokol_SOURCE_DIR}/sokol_app.m
)
target_link_libraries(sokol PRIVATE imgui)
target_include_directories(sokol INTERFACE ${sokol_SOURCE_DIR})
