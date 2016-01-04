cmake_minimum_required(VERSION 3.3)
project(cpp-sandbox VERSION 0.0.1 LANGUAGES CXX C)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic-errors -Werror")

find_package(GTest 1.7 EXACT REQUIRED)
find_package(Threads REQUIRED)

include(CTest)

add_subdirectory(boost)
