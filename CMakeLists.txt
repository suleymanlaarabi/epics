cmake_minimum_required(VERSION 3.15)

project(epics LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

if(ENABLE_ASAN)
    message(STATUS "Building with AddressSanitizer")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address)
endif()

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.1
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

add_subdirectory(libs/datastructure)
add_subdirectory(libs/ecs)
add_subdirectory(libs/rayflect)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE ecs rayflect datastructure  SFML::Graphics)

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/ecs/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/rayflect/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/datastructure/*.cpp
)

add_executable(unit_tests ${TEST_SOURCES})

find_library(CRITERION_LIB criterion PATHS /opt/homebrew/lib /usr/local/lib REQUIRED)

target_include_directories(unit_tests SYSTEM PRIVATE /opt/homebrew/include /usr/local/include)
target_link_libraries(unit_tests PRIVATE ${CRITERION_LIB} ecs rayflect datastructure SFML::Graphics)

enable_testing()
add_test(NAME run_unit_tests COMMAND unit_tests)
