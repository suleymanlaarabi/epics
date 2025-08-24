#include "ecs_entity.hpp"
#include <cassert>
#include <ecs_world.hpp>
#include <sparseset.hpp>


struct Position {
    float x, y;
};

struct Velocity {
    float x, y;
};

int main() {
    ecs::World world;

    ecs::Entity entity = world.entity();

    world.add(entity, Position {10, 10});
    world.add(entity, Velocity {1, 1});

    world.system<Position, Velocity>([](Position &pos, Velocity &vel) {
        pos.x += vel.x;
        pos.y += vel.y;
    });

    world.progress();
}
