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

struct Player {};

struct GravityPlugin : ecs::Plugin {

    static void positionSystem(ecs::u32 count, Position *pos) {
        for (ecs::u32 i = 0; i < count; ++i) {
            puts("ok");
        }
    }

    void build(ecs::World &world) {

        ecs::QueryBuilder(&world)
            .childOf<MainScene>()
            .without<Player>()
            .iter<Position>(this->positionSystem);


        ecs::QueryBuilder(&world)
            .childOf<MainScene>()
            .without<Player>()
            .iter<Position>(this->positionSystem);

        world.debug();

    }

};

int main() {
    ecs::World world;

    world.plugin(GravityPlugin());

    ecs::Entity player = world.entity();
    world.childOf<MainScene>(player);
    world.set(player, Position {10});

    world.progress();
}
