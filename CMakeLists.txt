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

#json
find_package(nlohmann_json CONFIG REQUIRED)

file(GLOB SOURCES "src/*.cpp" "src/mappers/*.cpp" "src/ui/*.cpp" "src/platform/*.cpp" "src/settings/*.cpp")
add_library(nesemul_lib STATIC ${SOURCES})
target_compile_options(nesemul_lib PUBLIC /std:c++latest /Z7 /Zc:preprocessor /W4 /WX /wd4100)
target_link_options(nesemul_lib PUBLIC /DEBUG:FASTLINK)
target_link_libraries(nesemul_lib sfml-system sfml-graphics sfml-window)
target_link_libraries(nesemul_lib imgui::imgui ImGui-SFML::ImGui-SFML)
target_link_libraries(nesemul_lib fmt::fmt fmt::fmt-header-only)
target_link_libraries(nesemul_lib nlohmann_json::nlohmann_json)

add_executable(nesemul_exe "exe/main.cpp")
target_link_libraries(nesemul_exe nesemul_lib)

include (tests/CMakeLists.txt)