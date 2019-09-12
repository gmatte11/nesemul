cmake_minimum_required(VERSION 3.6)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

project(nesemul)

#set(SFML_STATIC_LIBRARIES TRUE)
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
include_directories(${SFML_INCLUDE_DIRS} "src/")

file(GLOB SOURCES "src/*.cpp" "src/mappers/*.cpp")
add_executable(nesemul ${SOURCES})

target_compile_options(nesemul PUBLIC /std:c++17 /Z7)
target_link_options(nesemul PUBLIC /DEBUG:FASTLINK)
target_link_libraries(nesemul sfml-system sfml-graphics sfml-window)