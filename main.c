#include "ecs/ecs_types.h"
#include "ecs/ecs_world.h"
#include "ecs_system.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    float x, y;
} Position;

typedef struct {
    float x, y;
} Velocity;

ECS_COMPONENT_DEFINE(Position);
ECS_COMPONENT_DEFINE(Velocity);

void positions(ecs_iter *it) {
    for (uint32_t i = 0; i < it->count; i++) {
        puts("one");
    }
}

void positions2(ecs_iter *it) {
    for (uint32_t i = 0; i < it->count; i++) {
        puts("one");
    }
}

int main() {
    ecs_world *world = ecs_init();
    ECS_REGISTER_COMPONENT(world, Position);
    ECS_REGISTER_COMPONENT(world, Velocity);

    ecs_entity parent = ecs_new(world);
    ecs_entity enfant = ecs_new(world);
    ecs_entity enfant2 = ecs_new(world);

    ecs_entity ChildOf = ecs_new(world);

    ecs_add_pair(world, enfant, ChildOf, parent);
    ecs_add_pair(world, enfant2, ChildOf, parent);

    ecs_type query = ECS_TYPE(ecs_pair(ChildOf, parent).value);
    ecs_type query_mut = ECS_TYPE();
    ecs_register_system(world, positions, &query_mut, &query);

    ecs_progress(world);
}
