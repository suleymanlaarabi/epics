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

    ecs::Entity MainScene = world.entity();
    ecs::Entity enemy = world.entity();


    world.systemIter<Position, Velocity>([](ecs::u32 count, Position *pos, Velocity *vel) {
        for (ecs::u32 i = 0; i < count; i++) {
            pos[i].x += vel[i].x;
            pos[i].y += vel[i].y;
        }
    });

    world.system<Position, Velocity>([](ecs::EcsIter<Position, Velocity> iter) {
        for (auto [pos, vel] : iter) {
            pos.x += vel.x;
            pos.y += vel.y;
        }
    });


    ecs::Entity entity = world.entity();
    world.set(entity, Position {0, 0});
    world.set(entity, Velocity {1, 1});
    world.add(entity, enemy);
    world.childOf(entity, MainScene);


    world.progess();
    world.progess();
}
