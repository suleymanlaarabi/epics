#include "ecs_observer.h"
#include "ecs_types.h"
#include <criterion/criterion.h>
#include <ecs_world.h>
#include <stdio.h>

typedef struct {
    int x, y;
} Position;

Test(world, init_creates_valid_world) {
    ecs_world *world = ecs_init();
    cr_assert_not_null(world, "ecs_init must return a valid pointer");
}

Test(world, new_generates_unique_entities) {
    ecs_world *world = ecs_init();
    ecs_entity e1 = ecs_new(world);
    ecs_entity e2 = ecs_new(world);
    cr_assert(e1.value != e2.value, "Two calls to ecs_new must return distinct entities");
}

Test(world, add_component_size_registers_component) {
    ecs_world *world = ecs_init();
    ecs_entity comp = ecs_new(world);
    ecs_add_component_size(world, comp, sizeof(Position));

    ecs_entity player = ecs_new(world);
    ecs_add(world, player, comp);
    cr_assert(ecs_has(world, player, comp),
              "The player should have the added component");
}

Test(world, insert_stores_and_retrieves_data) {
    ecs_world *world = ecs_init();
    ecs_entity player = ecs_new(world);
    ecs_entity position = ecs_new(world);

    ecs_add_component_size(world, position, sizeof(Position));
    ecs_add(world, player, position);

    Position p = {10, 20};
    ecs_insert(world, player, position, &p);

    Position *stored = ecs_get(world, player, position);
    cr_assert_not_null(stored, "ecs_get must return a valid pointer");
    cr_expect_eq(stored->x, 10, "x should be 10");
    cr_expect_eq(stored->y, 20, "y should be 20");
}

Test(world, has_returns_false_for_missing_component) {
    ecs_world *world = ecs_init();
    ecs_entity player = ecs_new(world);
    ecs_entity position = ecs_new(world);

    ecs_add_component_size(world, position, sizeof(Position));

    cr_assert_not(ecs_has(world, player, position),
                  "ecs_has must return false if the player lacks the component");
}
typedef struct { int hp;   } Health;

Test(world, components_sorted_functional) {
    ecs_world *world = ecs_init();

    ecs_entity compPos = ecs_new(world);
    ecs_entity compHp  = ecs_new(world);

    ecs_add_component_size(world, compPos, sizeof(Position));
    ecs_add_component_size(world, compHp,  sizeof(Health));

    ecs_entity player = ecs_new(world);
    ecs_add(world, player, compHp);
    ecs_add(world, player, compPos);

    Position p = {42, 24};
    Health   h = {100};
    ecs_insert(world, player, compPos, &p);
    ecs_insert(world, player, compHp,  &h);

    Position *gotP = ecs_get(world, player, compPos);
    Health *gotH = ecs_get(world, player, compHp);

    cr_assert_not_null(gotP, "ecs_get must return Position if sorting works");
    cr_assert_not_null(gotH, "ecs_get must return Health if sorting works");
    cr_expect_eq(gotP->x, 42);
    cr_expect_eq(gotP->y, 24);
    cr_expect_eq(gotH->hp, 100);
}

Test(world, insert_overwrites_existing_component_data) {
    ecs_world *world = ecs_init();
    ecs_entity player = ecs_new(world);
    ecs_entity position = ecs_new(world);

    ecs_add_component_size(world, position, sizeof(Position));
    ecs_add(world, player, position);

    Position p1 = {1, 2};
    Position p2 = {99, 100};
    ecs_insert(world, player, position, &p1);
    ecs_insert(world, player, position, &p2);

    Position *stored = ecs_get(world, player, position);
    cr_assert_not_null(stored, "ecs_get must still return valid data after overwrite");
    cr_expect_eq(stored->x, 99, "x should be updated to 99");
    cr_expect_eq(stored->y, 100, "y should be updated to 100");
}

Test(world, remove_component) {
    ecs_world *world = ecs_init();
    ecs_entity player = ecs_new(world);
    ecs_entity position = ecs_new(world);

    ecs_add_component_size(world, position, sizeof(Position));
    ecs_add(world, player, position);

    Position p1 = {1, 2};
    Position p2 = {99, 100};
    ecs_insert(world, player, position, &p1);
    ecs_insert(world, player, position, &p2);

    cr_expect(ecs_has(world, player, position));

    ecs_remove(world, player, position);

    cr_expect(!ecs_has(world, player, position));
}

ecs_entity position;

void remove_added_position(ecs_world *world, ecs_entity entity) {
    ecs_remove(world, entity, position);
}

Test(world, ecs_observer_add) {
    ecs_world *world = ecs_init();

    ecs_entity player = ecs_new(world);
    position = ecs_new(world);

    ecs_add_observer(world, position, ECS_OBSERVER_ON_ADD, remove_added_position);
    ecs_add(world, player, position);
    cr_expect(!ecs_has(world, player, position));
    ecs_add(world, player, position);
    cr_expect(!ecs_has(world, player, position));

}


Test(world, ecs_remove_observer) {
    ecs_world *world = ecs_init();

    ecs_entity player = ecs_new(world);
    position = ecs_new(world);

    ecs_add_observer(world, position, ECS_OBSERVER_ON_ADD, remove_added_position);
    ecs_add(world, player, position);
    cr_expect(!ecs_has(world, player, position));
    ecs_remove_observer(world, position, ECS_OBSERVER_ON_ADD, remove_added_position);
    ecs_add(world, player, position);
    cr_expect(ecs_has(world, player, position));
}
