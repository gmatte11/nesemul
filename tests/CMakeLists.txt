#Catch2
find_package(Catch2 CONFIG REQUIRED)

file(GLOB TEST_SOURCES "tests/*.cpp")
add_executable(nesemul_tests ${TEST_SOURCES})
target_link_libraries(nesemul_tests nesemul_lib)
target_link_libraries(nesemul_lib Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(nesemul_tests)