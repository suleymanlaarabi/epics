#include "ecs_world.hpp"
#include <criterion/criterion.h>

struct Position {
    float x, y;
};

Test(entity, entity_component) {
    ecs::World world;
    ecs::Entity entity = world.entity();

    cr_assert(!world.has<Position>(entity));

    world.add<Position>(entity);
    cr_assert(world.has<Position>(entity));

    world.remove<Position>(entity);
    cr_assert(!world.has<Position>(entity));
}

Test(entity, entity_component_set) {
    ecs::World world;

    ecs::Entity entity = world.entity();
    world.entity();
    world.entity();

    world.set<Position>(entity, {10.0f, 20.0f});
    Position &pos = world.get<Position>(entity);

    cr_assert(pos.x == 10.0f);
    cr_assert(pos.y == 20.0f);

    world.remove<Position>(entity);
    cr_assert(!world.has<Position>(entity));

    ecs::Entity entity2 = world.entity();
    ecs::Entity entity3 = world.entity();
    world.entity();

    world.set(entity2, Position{5.0f, 15.0f});
    world.set(entity3, Position{15.0f, 5.0f});

    Position &pos2 = world.get<Position>(entity2);
    Position &pos3 = world.get<Position>(entity3);

    cr_assert(pos2.x == 5.0f);
    cr_assert(pos2.y == 15.0f);
    cr_assert(pos3.x == 15.0f);
    cr_assert(pos3.y == 5.0f);
}
