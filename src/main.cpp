#include "ecs/world.hpp"
#include <cassert>
#include <cstddef>
#include <stdio.h>


struct Position { double x, y; };
struct Velocity { float dx, dy; };

int main() {
    ecs::World world;

    ecs::Entity entity = world.entity();

    assert(entity == 1);

    world.addComponent<Position>(entity);

    assert(world.hasComponent<Position>(entity));

    world.removeComponent<Position>(entity);

    assert(!world.hasComponent<Position>(entity));

    world.setComponent(entity, Position {10, 0});

    assert(world.hasComponent<Position>(entity));

    Position *pos = world.getComponent<Position>(entity);

    assert(pos->x == 10 && pos->y == 0);
}
