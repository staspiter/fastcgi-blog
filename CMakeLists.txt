cmake_minimum_required(VERSION 3.13)
project(fastcgi_blog)

set(CMAKE_CXX_STANDARD 20)

include_directories(libraries/fcgi2/include)

add_executable(fastcgi_blog main.cpp)

target_link_libraries(fastcgi_blog fcgi fcgi++)