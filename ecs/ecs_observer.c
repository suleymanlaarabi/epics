#include "ecs_observer.h"
#include "ecs_types.h"
#include "ecs_vec.h"
#include "ecs_world.h"
#include <stdint.h>


void init_world_observer(ecs_world *world) {
    ecs_entity observer = ecs_new(world);
    ecs_add_component_size(world, observer, sizeof(ecs_observer));

    world->builtin_component.observer = observer;
}

void init_observer(ecs_observer *observer) {
    ecs_vec_init(&observer->on_add, sizeof(ecs_observer_call));
    ecs_vec_init(&observer->on_remove, sizeof(ecs_observer_call));
}


void ecs_add_observer(ecs_world *world, ecs_entity component, ecs_observer_type type, ecs_observer_call call) {
    if (!ecs_has(world, component, world->builtin_component.observer)) {
        ecs_add(world, component, world->builtin_component.observer);
        ecs_observer observer;
        init_observer(&observer);
        ecs_insert(world, component, world->builtin_component.observer, &observer);
    }

    ecs_observer *observer = ecs_get(world, component, world->builtin_component.observer);

    if (type == ECS_OBSERVER_ON_ADD) {
        ecs_vec_push(&observer->on_add, &call);
    } else {
        ecs_vec_push(&observer->on_remove, &call);
    }
}

void ecs_remove_observer(ecs_world *world, ecs_entity component, ecs_observer_type type, ecs_observer_call call) {
    if (!ecs_has(world, component, world->builtin_component.observer)) {
        return;
    }
    ecs_observer *observer = ecs_get(world, component, world->builtin_component.observer);
    ecs_vec *vec = NULL;
    ecs_observer_call *calls = NULL;
    uint32_t count = UINT32_MAX;

    if (type == ECS_OBSERVER_ON_ADD) {
        calls = observer->on_add.data;
        count = observer->on_add.count;
        vec = &observer->on_add;
    } else {
        calls = observer->on_remove.data;
        count = observer->on_remove.count;
        vec = &observer->on_remove;
    }

    for (uint32_t i = 0; i < count; i++) {
        if (calls[i] == call) {
            ecs_vec_remove_fast(vec, i);
        }
    }
}
