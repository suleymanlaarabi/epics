cmake_minimum_required(VERSION 3.15)

project(epics LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

if(ENABLE_ASAN)
    message(STATUS "Building with AddressSanitizer")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address)
endif()

add_subdirectory(libs/datastructure)
add_subdirectory(libs/ecs)
add_subdirectory(libs/rayflect)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ecs rayflect datastructure)

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/ecs/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/rayflect/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/datastructure/*.cpp
)

add_executable(unit_tests ${TEST_SOURCES})

find_library(CRITERION_LIB criterion PATHS /opt/homebrew/lib /usr/local/lib REQUIRED)

target_include_directories(unit_tests SYSTEM PRIVATE /opt/homebrew/include /usr/local/include)
target_link_libraries(unit_tests PRIVATE ${CRITERION_LIB} ecs rayflect datastructure)

enable_testing()
add_test(NAME run_unit_tests COMMAND unit_tests)
