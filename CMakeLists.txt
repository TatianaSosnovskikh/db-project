cmake_minimum_required(VERSION 3.16)
project(ImGuiDatabase VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com
    GIT_TAG v1.90.4
)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com
    GIT_TAG 3.4
    GIT_SUBMODULES ""
)

FetchContent_MakeAvailable(imgui glfw)

find_package(OpenGL REQUIRED)

set(PROJECT_SOURCES
    main.cpp
    database_core.cpp
    database_core.h
)

list(APPEND PROJECT_SOURCES 
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${imgui_SOURCE_DIR} 
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    glfw 
    OpenGL::GL
)
