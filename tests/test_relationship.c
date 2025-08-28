#include "ecs_types.h"
#include <criterion/criterion.h>
#include <ecs_world.h>
#include <stdio.h>

int relationship_first_test_count = 0;
void relationship_first_test_system(ecs_iter *it) {
    for (uint32_t i = 0; i < it->count; i++) {
        relationship_first_test_count++;
    }
}

Test(relationship, test_relationship) {
    ecs_world *world = ecs_init();

    ecs_entity parent = ecs_new(world);
    ecs_entity enfant = ecs_new(world);
    ecs_entity enfant2 = ecs_new(world);

    ecs_entity ChildOf = ecs_new(world);

    ecs_add_pair(world, enfant, ChildOf, parent);
    ecs_add_pair(world, enfant2, ChildOf, parent);

    ecs_type query = ECS_TYPE(ecs_pair(ChildOf, parent).value);
    ecs_type query_mut = ECS_TYPE();
    ecs_register_system(world, relationship_first_test_system, &query_mut, &query);

    ecs_progress(world);
    cr_assert_eq(relationship_first_test_count, 2);
}
