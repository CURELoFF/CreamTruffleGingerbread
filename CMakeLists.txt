cmake_minimum_required(VERSION 3.20)

project(CreamTruffleGingerbread
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(CreamTruffleGingerbread
    src/main.cpp
    src/core/Window.cpp
    src/core/Application.cpp
    src/core/Renderer.cpp
    src/gameobject/EntityEnemy.cpp
    src/gameobject/EntityPlayer.cpp
)

find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)
target_link_libraries(CreamTruffleGingerbread PRIVATE SDL3::SDL3 SDL3_image::SDL3_image)
target_compile_options(CreamTruffleGingerbread PRIVATE /utf-8)
