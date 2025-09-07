#include "ecs_type.hpp"
#include <cassert>
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

int main() {
    ecs::World world;

    ecs::Entity player = world.entity();
    ecs::Entity enemy = world.entity();

    world.set<Position>(player, {100.0f, 50.0f});
    world.set<Velocity>(player, {5.0f, -2.0f});
    world.set<Health>(player, {100});

    world.set<Position>(enemy, {200.0f, 150.0f});
    world.set<Health>(enemy, {50});


    Position &player_pos = world.get<Position>(player);
    Position &enemy_pos = world.get<Position>(enemy);

    world.remove<Velocity>(player);
}
