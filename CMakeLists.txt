cmake_minimum_required(VERSION 3.16)
project(ecs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra -pedantic")

file(GLOB_RECURSE SRC "src/*.cpp")

add_executable(main ${SRC})

target_include_directories(main PRIVATE src)
