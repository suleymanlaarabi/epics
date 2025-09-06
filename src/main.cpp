#include "ecs_system.hpp"
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

    ecs::Entity parent = world.entity();

    ecs::QueryBuilder(&world)
        .childOf(parent)
        .each<Position, Velocity>([](ecs::EcsIter<Position, Velocity> iter) {
            for (auto [pos, vel] : iter) {
                pos.x += vel.x;
                pos.y += vel.y;
            }
        });

    ecs::Entity entity = world.entity();
    world.add<Position>(entity);
    world.add<Velocity>(entity);
    world.childOf(entity, parent);

    world.progess();
}
