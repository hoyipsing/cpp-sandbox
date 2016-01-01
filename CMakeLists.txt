cmake_minimum_required(VERSION 3.2)
project(cpp-sandbox VERSION 0.0.1 LANGUAGES CXX)

find_package(GTest 1.7 EXACT REQUIRED)
find_package(Threads REQUIRED)

include(CTest)

add_subdirectory(boost)
