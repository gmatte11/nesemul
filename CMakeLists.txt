cmake_minimum_required(VERSION 3.6)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

project(nesemul)

# SFML
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
include_directories(${SFML_INCLUDE_DIRS} "src/")

# fmt
find_package(fmt CONFIG REQUIRED)

file(GLOB SOURCES "src/*.cpp" "src/mappers/*.cpp")
add_executable(nesemul ${SOURCES})

target_compile_options(nesemul PUBLIC /std:c++17 /Z7)
target_link_options(nesemul PUBLIC /DEBUG:FASTLINK)
target_link_libraries(nesemul PRIVATE sfml-system sfml-graphics sfml-window fmt::fmt fmt::fmt-header-only)