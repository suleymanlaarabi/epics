#include "ecs_type.hpp"
#include <ecs_world.hpp>

struct Position {
    float x, y;
};

struct Velocity {
    float x, y;
};

int main() {
    ecs::World world;



    ecs::Entity entity = world.entity();
    world.set(entity, Position {10, 10});
    world.set(entity, Velocity {1, 1});

    Position &pos = world.get<Position>(entity);

    std::cout << "Position: (" << pos.x << ", " << pos.y << ")" << std::endl;
}
