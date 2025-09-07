#include "ecs_system.hpp"
#include "ecs_type.hpp"
#include <cassert>
#include <cstdio>
#include <ecs_world.hpp>

struct Position {
    float x, y;
};

struct Velocity {
    float x, y;
};

struct Health {
    int value;
};

struct MainScene {};

struct GravityPlugin : ecs::Plugin {
    int live = 0;

    void build(ecs::World &world) {
        ecs::QueryBuilder(&world)
            .childOf<MainScene>()
            .iter<Position>([](ecs::u32 count, Position *pos) {
                for (ecs::u32 i = 0; i < count; ++i) {
                    puts("ok");
                }
            });
    }
};

int main() {
    ecs::World world;

    world.plugin(GravityPlugin());
    world.plugin(GravityPlugin());

    ecs::Entity player = world.entity();
    world.childOf<MainScene>(player);
    world.set(player, Position {10});

    world.progress();
}
