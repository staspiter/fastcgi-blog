cmake_minimum_required(VERSION 3.17)
project(fastcgi_blog)

set(CMAKE_CXX_STANDARD 20)

include_directories(libraries/fcgi2/include)

add_executable(fastcgi_blog main.cpp)