#ifndef  ECS_OBSERVER_H
    #define ECS_OBSERVER_H
    #include "ecs_types.h"
    #include "ecs_world.h"
    #include <stddef.h>
    #include <stdint.h>

typedef void (*ecs_observer_call) (ecs_world *world, ecs_entity entity);

typedef enum {
    ECS_OBSERVER_ON_ADD,
    ECS_OBSERVER_ON_REMOVE
} ecs_observer_type;

typedef struct {
    ecs_vec on_add;
    ecs_vec on_remove;
} ecs_observer;

void init_observer(ecs_observer *world);
void init_world_observer(ecs_world *world);
void ecs_add_observer(ecs_world *world, ecs_entity component, ecs_observer_type type, ecs_observer_call call);
void ecs_remove_observer(ecs_world *world, ecs_entity component, ecs_observer_type type, ecs_observer_call call);

#endif
