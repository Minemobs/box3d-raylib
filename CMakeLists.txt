cmake_minimum_required(VERSION 3.10)
project(box3d-raylib C)
set(CMAKE_C_STANDARD 23)

# Adding Raylib
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG "65abee1cbade6bf7edf55da6eb1eed6980aa754b"
    GIT_PROGRESS TRUE
    GIT_SHALLOW 1
)

FetchContent_MakeAvailable(raylib)

FetchContent_Declare(box3d
    GIT_REPOSITORY "https://github.com/erincatto/box3d.git"
    GIT_TAG "v0.1.0"
    GIT_PROGRESS TRUE
    GIT_SHALLOW 1
)

FetchContent_MakeAvailable(box3d)

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/sources/*.c")
set(PROJECT_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/sources/")

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE})
target_link_libraries(${PROJECT_NAME} PRIVATE raylib box3d::box3d)

if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
endif()

set(SANITIZER_FLAGS
    -fsanitize=address
    -fsanitize=leak
    -fsanitize=undefined
)

set(DEBUG_FLAGS
    -g
    -O0
    -Wall
)

set(RELEASE_FLAGS
    -O3
    -DNDEBUG
)

target_compile_options(${PROJECT_NAME}
    PRIVATE
    $<$<CONFIG:Debug>:${DEBUG_FLAGS}>
    $<$<CONFIG:Release>:${RELEASE_FLAGS}>
)

target_link_options(
    ${PROJECT_NAME}
    PRIVATE
    $<$<CONFIG:Debug>:${SANITIZER_FLAGS}>
)

target_compile_definitions(
    ${PROJECT_NAME}
    PUBLIC
    ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/"
)
