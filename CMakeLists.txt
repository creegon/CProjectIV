cmake_minimum_required(VERSION 3.14)

project(demo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(demo
    test.cpp
)

target_include_directories(demo
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)