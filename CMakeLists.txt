cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

include_directories(
        libraries/fcgi2/include
        libraries/rapidjson/include
        )

add_subdirectory(libraries/googletest)

project(fblog)
add_executable(fblog main.cpp Tree.h Utils.h Generator.h)
target_link_libraries(fblog fcgi fcgi++)

project(tests)
add_executable(tests tests.cpp Tree.h Utils.h Generator.h)
target_link_libraries(tests gtest gtest_main)
target_compile_definitions(tests PUBLIC tests)

if(UNIX AND NOT APPLE)
        target_link_libraries(fblog stdc++fs)
        target_link_libraries(tests stdc++fs)
endif()