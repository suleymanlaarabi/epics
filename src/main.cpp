#include "ecs_type.hpp"
#include "rayflect.hpp"
#include <cassert>
#include <cstdio>
#include <ecs_world.hpp>
#include <ecs_system.hpp>

#include <SFML/Graphics.hpp>
#include <iostream>

struct Position {
    float x;
    float y;
};
struct Velocity {
    float x;
    float y;
};
struct Player {};
struct MainScene{};

int main() {
    ecs::World world;

    world.system()
        .read<Position>()
        .each([](ecs::EcsIter<const Position> iter){
            for(auto [pos] : iter) {
                puts("ok");
            }
        });


    std::cout << "ID: " << rayflect::getComponentID<Position>() << std::endl;
    std::cout << "ID: " << rayflect::getComponentID<const Position>() << std::endl;

    ecs::Entity entity = world.entity();
    world.add<Position>(entity);

    world.progress();
}
