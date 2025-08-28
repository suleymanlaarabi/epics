#include "ecs_world.h"
#include "datastructure/ecs_sparseset.h"
#include "datastructure/ecs_vec.h"
#include "datastructure/ecs_vec_map.h"
#include "ecs_archetype.h"
#include "ecs_entity.h"
#include "ecs_observer.h"
#include "ecs_system.h"
#include "ecs_types.h"
#include "ecs_vec_sort.h"
#include "include/ecs_config.h"
#include <stdint.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

ecs_world *ecs_init(void) {
    ecs_world *world = malloc(sizeof(ecs_world));
    ecs_init_entity_manager(&world->entity_manager);
    world->type_archetype = ecs_hashmap_new(sizeof(ecs_archetype), 1000);
    ecs_vec_init(&world->systems_batch, sizeof(ecs_vec));
    ecs_sparseset_init(&world->component_store, sizeof(ecs_component_record));
    ecs_vec_init(&world->g_jobs, sizeof(system_job));
    ecs_vec_init(&world->g_threads, sizeof(pthread_t));
    world->g_initialized = 1;
    init_world_observer(world);
    return world;
}

ecs_archetype *new_archetype(ecs_world *world, ecs_type *type) {
    ecs_archetype *archetype = ecs_hashmap_ensure(&world->type_archetype, type);
    ecs_archetype_init(archetype);
    ecs_entity *components = type->data;

    for (uint32_t i = 0; i < type->count; i++) {
        if (ecs_sparseset_contains(&world->component_store, components[i].value)) {
            ecs_archetype_add_col(
                archetype, components[i],
                GET_COMPONENT_RECORD(world, components[i])->size
            );
        } else {
            ecs_archetype_add_singleton(archetype, components[i]);
        }
    }

    ecs_vec *batchs = world->systems_batch.data;
    uint32_t len = world->systems_batch.count;

    for (uint32_t i = 0; i < len; i++) {
        ecs_system *systems = batchs[i].data;
        uint32_t count = batchs[i].count;

        for (uint32_t j = 0; j < count; j++) {
            ecs_system *system = &systems[i];
            if (ecs_vec_is_subset(&system->matches, type)) {
                ecs_vec_push(&system->matches, &archetype);
            }
        }
    }

    return archetype;
}

void migrate_entity(ecs_world *world, uint32_t row, ecs_archetype *from, ecs_archetype *to) {
    ecs_entity *from_components = from->type.data;
    ecs_entity *to_components = to->type.data;

    for (uint32_t from_i = 0, to_i = 0; from_i < from->type.count && to_i < to->type.count; from_i++, to_i++) {
        if (from_components[from_i].value == to_components[to_i].value) {
            size_t size = GET_COMPONENT_RECORD(world, from_components[from_i])->size;
            if (size) {
                void *from_row = ECS_VEC_GET(void,
                    ((ecs_vec *) ecs_sparseset_get(&from->cols, from_components[from_i].value)),
                    row
                );
                void *to_row = ECS_VEC_GET(void,
                    ((ecs_vec *) ecs_sparseset_get(&to->cols, to_components[to_i].value)),
                    row
                );
                memcpy(to_row, from_row, size);
            }
        }
    }
}

void ecs_add(ecs_world *world, ecs_entity e, ecs_entity b) {

    if (ecs_has(world, e, b)) {
        return;
    }

    ecs_entity_record *record = ECS_GET_RECORD(world, e);
    ecs_archetype *archetype = record->archetype;
    ecs_archetype *newArchetype = NULL;

    if (archetype) {
        newArchetype = ecs_sparseset_get(&archetype->add_edge, b.value);

        if (ECS_UNLIKELY(!newArchetype)) {
            ecs_type type;
            ecs_vec_copy(&archetype->type, &type);
            ecs_vec_push(&type, &b.value);
            ecs_vec_sort_u64(&type);
            if (!ecs_hashmap_get_ptr(&world->type_archetype, &type, (void *) &newArchetype)) {
                newArchetype = new_archetype(world, &type);
            }
            ecs_sparseset_add(&archetype->add_edge, b.value, newArchetype);
        }

        record->row = ecs_archetype_add_entity(newArchetype, e);
        migrate_entity(world, record->row, archetype, newArchetype);
        ecs_archetype_remove_result result = ecs_archetype_remove_entity(archetype, record->row);
        if (result.swapped_entity_new_row != UINT32_MAX) {
            ECS_VEC_GET(
                ecs_entity_record,
                &world->entity_manager.entity_record,
                result.swapped_entity_new_row
            )->row = result.swapped_entity_new_row;
        }
    } else {
        ecs_type type = (ecs_type) ECS_TYPE(b.value);

        if (!ecs_hashmap_get_ptr(&world->type_archetype, &type, ((void *) &newArchetype))) {
            newArchetype = new_archetype(world, &type);
        }
        record->row = ecs_archetype_add_entity(newArchetype, e);
    }
    record->archetype = newArchetype;

    if (ecs_has(world, b, world->builtin_component.observer)) {
        ecs_vec *observer = &((ecs_observer *) ecs_get(world, b, world->builtin_component.observer))->on_add;
        ecs_observer_call *calls = observer->data;
        for (uint32_t i = 0; i < observer->count; i++) {
            calls[i](world, e);
        }
    }
}

void ecs_remove(ecs_world *world, ecs_entity e, ecs_entity b) {

    if (!ecs_has(world, e, b)) {
        return;
    }

    if (ecs_has(world, b, world->builtin_component.observer)) {
        ecs_vec *observer = &((ecs_observer *) ecs_get(world, b, world->builtin_component.observer))->on_remove;
        ecs_observer_call *calls = observer->data;
        for (uint32_t i = 0; i < observer->count; i++) {
            calls[i](world, e);
        }
    }

    ecs_entity_record *record = ECS_GET_RECORD(world, e);
    ecs_archetype *archetype = record->archetype;
    ecs_archetype *newArchetype = NULL;

    newArchetype = ecs_sparseset_get(&archetype->remove_edge, b.value);

    if (ECS_UNLIKELY(!newArchetype)) {
        ecs_type type;
        ecs_vec_copy(&archetype->type, &type);
        for (uint32_t i = 0; i < type.count; i++) {
            if (((uint32_t *) type.data)[i] == b.value) {
                ecs_vec_remove_ordered(&type, i);
                break;
            }
        }
        if (!ecs_hashmap_get_ptr(&world->type_archetype, &type, (void *) &newArchetype)) {
            newArchetype = new_archetype(world, &type);
        }

        ecs_sparseset_add(&archetype->remove_edge, b.value, newArchetype);
    }


    record->row = ecs_archetype_add_entity(newArchetype, e);
    migrate_entity(world, record->row, archetype, newArchetype);
    record->archetype = newArchetype;
    ecs_archetype_remove_result result = ecs_archetype_remove_entity(archetype, record->row);

    if (ECS_UNLIKELY(result.swapped_entity_new_row != UINT32_MAX)) {
        ECS_VEC_GET(
            ecs_entity_record,
            &world->entity_manager.entity_record,
            result.swapped_entity_new_row
        )->row = result.swapped_entity_new_row;
    }
}


void ecs_add_pair(ecs_world *world, ecs_entity entity, ecs_entity relation, ecs_entity target) {
    ecs_entity relation_component = {
        .relation = {
            .relation = relation.index,
            .target = target.index
        }
    };

    ecs_add(world, entity, relation_component);
}

void ecs_register_system(ecs_world *world, ecs_system_func func, ecs_type *query_mut, ecs_type *query) {
    ecs_vec *batchs = world->systems_batch.data;
    uint32_t len = world->systems_batch.count;

    ecs_type type = ecs_vec_concat(query_mut, query);
    ecs_vec_sort_u64(&type);


    ecs_system system = {
        .type = type,
        .query = {0},
        .query_mut = {0},
        .func = func,
        .matches = {0}
    };

    ecs_vec_copy(&system.query_mut, query_mut);
    ecs_vec_copy(&system.query, query);
    ecs_vec_init(&system.matches, sizeof(ecs_archetype *));

    for (uint32_t i = 0; i < len; i++) {
        ecs_system *systems = batchs[i].data;
        uint32_t count = batchs[i].count;

        for (uint32_t j = 0; j < count; j++) {
            if (ecs_systems_are_compatible(
                &systems[i].query, &systems[i].query_mut,
                query, query_mut
            )) {
                ecs_vec_push(&batchs[i], &system);
            }
        }
    }

    ecs_vec batch = {0};
    ecs_vec_init(&batch, sizeof(ecs_system));


    size_t i = 0;
    ecs_hashmap_entry e;
    while (ecs_hashmap_next(&world->type_archetype, &i, &e)) {
        ecs_archetype *archetype = e.value;

        if (ecs_vec_is_subset(&system.type, &archetype->type)) {
            ecs_vec_push(&system.matches, &archetype);
        }
    }
    ecs_vec_push(&batch, &system);
    ecs_vec_push(&world->systems_batch, &batch);

}

void *run_system(void *arg) {
    system_job *job = arg;
    ecs_system *system = job->system;

    for (uint32_t k = 0; k < system->matches.count; k++) {
        ecs_archetype *archetype = ((ecs_archetype **) system->matches.data)[k];
        system->func(&(ecs_iter){
            .archetype = archetype,
            .count = archetype->entities.count
        });
    }

    return NULL;
}

static void ensure_capacity(ecs_world *world,  size_t count) {
    ecs_vec_ensure(&world->g_threads, count);
    ecs_vec_ensure(&world->g_jobs, count);
}


void ecs_progress(ecs_world *world) {
    ecs_vec *batchs = world->systems_batch.data;
    uint32_t len = world->systems_batch.count;

    for (uint32_t i = 0; i < len; i++) {
        ecs_system *systems = batchs[i].data;
        uint32_t count = batchs[i].count;

        ensure_capacity(world, count);

        world->g_threads.count = 0;
        world->g_jobs.count = 0;

        for (uint32_t j = 0; j < count; j++) {
            system_job job = {.system = &systems[j]};
            ecs_vec_push(&world->g_jobs, &job);
            pthread_t t;
            ecs_vec_push(&world->g_threads, &t);

            pthread_create(
                (pthread_t *)((char*)world->g_threads.data + j * world->g_threads.size),
                NULL, run_system,
                (system_job *)((char*)world->g_jobs.data + j * world->g_jobs.size)
            );
        }

        for (uint32_t j = 0; j < count; j++) {
            pthread_join(*(pthread_t *)((char*)world->g_threads.data + j * world->g_threads.size), NULL);
        }
    }
}
