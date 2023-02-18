include(FetchContent)

FetchContent_Declare(
    fmt
    OVERRIDE_FIND_PACKAGE
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG 9.1.0)
FetchContent_MakeAvailable(fmt)

set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "" FORCE)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
    GIT_TAG v1.10.0)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm
    GIT_TAG 0.9.9.8)
FetchContent_MakeAvailable(glm)

add_compile_definitions(ENTT_NOEXCEPTION)
FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG v3.11.1)
FetchContent_MakeAvailable(entt)

# use float instead of double for chipmunk; we don't need the extra precision,
# and it matches what glm and ImGui use by default.
add_compile_definitions(CP_USE_DOUBLES=0 CP_USE_CGTYPES=0)
FetchContent_Declare(
    chipmunk2d
    GIT_REPOSITORY https://github.com/slembcke/Chipmunk2D
    GIT_TAG Chipmunk-7.0.3)
FetchContent_MakeAvailable(chipmunk2d)
target_include_directories(chipmunk PUBLIC ${chipmunk_SOURCE_DIR}/include)
target_compile_definitions(chipmunk PUBLIC CP_USE_DOUBLES=0 CP_USE_CGTYPES=0)

add_subdirectory(imgui)
add_subdirectory(sokol)
add_subdirectory(stb)
