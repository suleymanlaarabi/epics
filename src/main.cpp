#include "ecs/component.hpp"
#include "ecs/world.hpp"
#include <cstddef>
#include <iostream>
#include <stdio.h>


struct Position { double x, y; };
struct Velocity { float dx, dy; };

int main() {
    ecs::World world;

    ecs::Entity entity = world.entity();


    std::cout << "Has component Position: " << world.hasComponent<Position>(entity) << "\n";

    world.addComponent<Position>(entity);

    std::cout << "Has component Position: " << world.hasComponent<Position>(entity) << "\n";

    std::cout << "Position ID: " << ecs::getComponentID<Position>() << "\n";
    std::cout << "Velocity ID: " << ecs::getComponentID<Velocity>() << "\n";
    std::cout << "Position ID: " << ecs::getComponentID<Position>() << "\n";
}
