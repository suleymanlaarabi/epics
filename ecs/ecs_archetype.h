#ifndef ECS_ARCHETYPE_H
    #define ECS_ARCHETYPE_H
    #include "datastructure/ecs_sparseset.h"
    #include "datastructure/ecs_vec.h"
    #include "ecs_types.h"
    #include <stdint.h>


typedef struct {
    uint32_t removed_entity_index;
    uint32_t swapped_entity_new_row;
} ecs_archetype_remove_result;

typedef struct {
    ecs_sparseset cols;
    ecs_vec entities;
    ecs_type type;

    ecs_sparseset add_edge;
    ecs_sparseset remove_edge;
} ecs_archetype;

void ecs_archetype_init(ecs_archetype *archetype);
void ecs_archetype_add_col(ecs_archetype *archetype, ecs_entity component, size_t size);
void ecs_archetype_add_singleton(ecs_archetype *archetype, ecs_entity component);
uint32_t ecs_archetype_add_entity(ecs_archetype *archetype, ecs_entity entity);
ecs_archetype_remove_result ecs_archetype_remove_entity(ecs_archetype *archetype, size_t row);

#endif
