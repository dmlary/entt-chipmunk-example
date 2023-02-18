FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb
    GIT_TAG 8b5f1f37b5b75829fc72d38e7b5d4bcbf8a26d55 # 2022-09-08
    CONFIGURE_COMMAND ""
    BUILD_COMMAND "")

FetchContent_GetProperties(stb)
if(NOT stb_POPULATED)
    FetchContent_Populate(stb)
endif()

configure_file(stb.cpp.in ${stb_SOURCE_DIR}/stb.cpp)
add_library(stb STATIC
    ${stb_SOURCE_DIR}/stb.cpp
)
target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})
