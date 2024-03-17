cmake_minimum_required(VERSION 3.6)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

project(nesemul)

# SFML
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
include_directories(${SFML_INCLUDE_DIRS} "src")

# imgui
find_package(imgui CONFIG REQUIRED)
find_package(imgui-sfml CONFIG REQUIRED)

# fmt
find_package(fmt CONFIG REQUIRED)

file(GLOB SOURCES "src/*.cpp" "src/mappers/*.cpp" "src/ui/*.cpp" "src/platform/*.cpp")
add_executable(nesemul ${SOURCES})

target_compile_options(nesemul PUBLIC /std:c++latest /Z7 /W4 /WX /wd4100)
target_link_options(nesemul PUBLIC /DEBUG:FASTLINK)
target_link_libraries(nesemul PRIVATE sfml-system sfml-graphics sfml-window)
target_link_libraries(nesemul PRIVATE imgui::imgui ImGui-SFML::ImGui-SFML)
target_link_libraries(nesemul PRIVATE fmt::fmt fmt::fmt-header-only)