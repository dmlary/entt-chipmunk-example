cmake_minimum_required(VERSION 3.24)

project(entt-chipmunk2d-imgui
    VERSION 0.1
    DESCRIPTION "sample implementation of chipmunk2d in entt, imgui editor"
    LANGUAGES C CXX)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(vendor)
add_subdirectory(src)

# Add "tags" target
set_source_files_properties(tags PROPERTIES GENERATED true)
add_custom_target(tags
    COMMAND ctags -R --c++-kinds=+p --fields=+iaS --extra=+q ./src ./vendor
        ./build/_deps/*-src
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Add "cscope" target
set_source_files_properties(cscope PROPERTIES GENERATED true)
add_custom_target(cscope
    COMMAND cscope -bqUR
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
