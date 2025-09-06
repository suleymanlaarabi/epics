file(GLOB ECS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(ecs STATIC ${ECS_SOURCES})

target_include_directories(ecs PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ecs PUBLIC datastructure rayflect)

if(ENABLE_ASAN)
    target_compile_options(ecs PUBLIC -fsanitize=address -fno-omit-frame-pointer -g)
    target_link_options(ecs PUBLIC -fsanitize=address)
endif()
