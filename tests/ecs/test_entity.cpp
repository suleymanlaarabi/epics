#include "ecs_entity.hpp"
#include "ecs_world.hpp"
#include <criterion/criterion.h>

struct Position {
    float x, y;
};

Test(entity, create_entity_id) {
    ecs::World world;

    ecs::Entity entity1 = world.entity();
    ecs::Entity entity2 = world.entity();
    ecs::Entity entity3 = world.entity();

    cr_assert(entity1.index == 0);
    cr_assert(entity1.gen == 0);

    cr_assert(entity2.index == 1);
    cr_assert(entity3.index != 1);

    cr_assert(world.isAlive(entity1));

    world.kill(entity1);
    cr_assert(!world.isAlive(entity1));

    ecs::Entity entity1gen2 = world.entity();
    cr_assert(entity1gen2.index == entity1.index);
    cr_assert(entity1gen2.gen == entity1.gen + 1);
}

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

    world.add<Position>(entity, {10.0f, 20.0f});
    Position &pos = world.get<Position>(entity);

    cr_assert(pos.x == 10.0f);
    cr_assert(pos.y == 20.0f);

    world.remove<Position>(entity);
    cr_assert(!world.has<Position>(entity));

    ecs::Entity entity2 = world.entity();
    ecs::Entity entity3 = world.entity();
    world.entity();

    world.add(entity2, Position{5.0f, 15.0f});
    world.add(entity3, Position{15.0f, 5.0f});

    Position &pos2 = world.get<Position>(entity2);
    Position &pos3 = world.get<Position>(entity3);

    cr_assert(pos2.x == 5.0f);
    cr_assert(pos2.y == 15.0f);
    cr_assert(pos3.x == 15.0f);
    cr_assert(pos3.y == 5.0f);
}
