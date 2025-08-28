#ifndef ECS_WORLD_H
    #define ECS_WORLD_H

    #include "datastructure/ecs_set.h"
    #include "datastructure/ecs_sparseset.h"
    #include "datastructure/ecs_vec.h"
    #include "ecs_archetype.h"
    #include "ecs_entity.h"
    #include "ecs_system.h"
    #include "ecs_types.h"
    #include "datastructure/ecs_vec_map.h"
    #include <stddef.h>
    #include <stdint.h>
    #include <stdio.h>
    #define ECS_TYPE(...) ECS_RAW_VEC(uint64_t, __VA_ARGS__)
    #define GET_COMPONENT_RECORD(world, component) ((ecs_component_record *) ecs_sparseset_get(&(world)->component_store, component.value))
    #define DEBUG_ENTITY(e) printf("Entity %d, generation %d\n", e.index, e.gen)
    #define ecs_field(it, component) ((ecs_vec *) ecs_sparseset_get(&(it)->archetype->cols, ecs_component(component).value))->data
    #define ecs_set(world, entity, component, ...) ecs_insert(world, entity, ecs_component(component), &(component) __VA_ARGS__);

    #include <pthread.h>
    #include <stdlib.h>
    #include <string.h>

typedef struct { ecs_system *system; } system_job;

typedef struct {
    size_t size;
    ecs_set archetype_set;
} ecs_component_record;

typedef struct {
    ecs_sparseset component_store;
    ecs_entity_manager entity_manager;
    ecs_hashmap type_archetype;
    ecs_system_batch systems_batch;

    ecs_vec g_threads;
    ecs_vec g_jobs;
    int g_initialized;

    struct {
        ecs_entity observer;
    } builtin_component;
} ecs_world;

ecs_world *ecs_init();
void ecs_add(ecs_world *world, ecs_entity e, ecs_entity b);
void ecs_remove(ecs_world *world, ecs_entity e, ecs_entity b);
void ecs_add_pair(ecs_world *world, ecs_entity entity, ecs_entity relation, ecs_entity target);
void ecs_register_system(ecs_world *world, ecs_system_func func, ecs_type *query_mut, ecs_type *query);
void ecs_progress(ecs_world *world);

static inline ecs_entity ecs_new(ecs_world *world) {
    return ecs_entity_manager_new_entity(&world->entity_manager);
}

static inline void ecs_kill(ecs_world *world, ecs_entity entity) {
    ecs_entity_manager_kill_entity(&world->entity_manager, entity.index);
}

static inline bool ecs_is_alive(ecs_world *world, ecs_entity entity) {
    return ecs_entity_manager_entity_is_alive(&world->entity_manager, entity);
}

static inline bool ecs_has(ecs_world *world, ecs_entity entity, ecs_entity component) {
    ecs_archetype *archetype = ECS_GET_RECORD(world, entity)->archetype;

    if (!archetype) {
        return false;
    }

    return ecs_sparseset_contains(&archetype->cols, component.value);
}

static inline void ecs_insert(ecs_world *world, ecs_entity entity, ecs_entity component, void *value) {

    if (!ecs_has(world, entity, component)) {
        ecs_add(world, entity, component);
    }

    ecs_entity_record *record = ECS_GET_RECORD(world, entity);
    ecs_vec *col = (ecs_vec *) ecs_sparseset_get(&record->archetype->cols, component.value);

    ecs_vec_set(col, record->row, value);
}

static inline void *ecs_get(ecs_world *world, ecs_entity entity, ecs_entity component) {
    ecs_entity_record *record = ECS_GET_RECORD(world, entity);
    ecs_vec *col = (ecs_vec *) ECS_SPARSESET_GET_DENSE(&record->archetype->cols, component.value);

    return ECS_VEC_GET(void, col, record->row);
}

static inline void ecs_add_component_size(ecs_world *world, ecs_entity entity, size_t size) {
    ecs_component_record record = {
        .size = size,
    };

    ecs_sparseset_add(&world->component_store, entity.value, &record);
}


#endif
