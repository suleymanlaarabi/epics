#ifndef ECS_ENTITY_H
    #define ECS_ENTITY_H
    #include "ecs_archetype.h"
    #include "datastructure/ecs_vec.h"
    #include "ecs_types.h"
    #include <stdint.h>
    #include <stdbool.h>
    #define ECS_GET_RECORD(world, e) ECS_VEC_GET(ecs_entity_record, &(world)->entity_manager.entity_record, e.index)

typedef struct {
    ecs_archetype *archetype;
    uint32_t row;
} ecs_entity_record;

typedef struct {
    ecs_vec generations;
    ecs_vec entity_record;
    ecs_vec available_entity;
} ecs_entity_manager;

void ecs_init_entity_manager(ecs_entity_manager *manager);
ecs_entity ecs_entity_manager_new_entity(ecs_entity_manager *manager);
bool ecs_entity_manager_entity_is_alive(ecs_entity_manager *manager, ecs_entity e);
void ecs_entity_manager_kill_entity(ecs_entity_manager *manager, uint32_t index);

#endif
