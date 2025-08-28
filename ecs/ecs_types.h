#ifndef ECS_TYPE_H
    #define ECS_TYPE_H
    #include <stdbool.h>
    #include <stddef.h>
    #include <stdint.h>
    #include <stdio.h>
    #include "datastructure/ecs_vec.h"

    #define ECS_COMPONENT_DEFINE(component) \
        ecs_component_define ECS_##component##ID = { \
            .size = sizeof(Position), \
            .entity = {0} \
        };

    #define ECS_REGISTER_COMPONENT(world, component) \
        ECS_##component##ID.entity = ecs_new(world);  \
        ecs_add_component_size(world, ECS_##component##ID.entity, sizeof(component))

    #define ecs_component(component) ECS_##component##ID.entity
    #define ecs_pair(component_relation, component_target) \
        ((ecs_entity) { \
            .relation = { \
                .relation = component_relation.index, \
                .target = component_target.index \
            } \
        })

typedef union {
    struct {
        uint32_t index;
        uint16_t gen;
        uint16_t flags;
    };
    struct {
        uint32_t relation;
        uint32_t target;
    } relation;
    uint64_t value;
} ecs_entity;



typedef struct {
    ecs_entity entity;
    size_t size;
} ecs_component_define;

typedef ecs_vec ecs_type;
typedef size_t ecs_size;
typedef uint64_t ecs_component_id ;

static inline bool ecs_vec_is_subset(const ecs_vec *subset, const ecs_vec *set) {
    if (subset->count > set->count) return false;

    const uint64_t *sub_ids = (const uint64_t*)subset->data;
    const uint64_t *set_ids = (const uint64_t*)set->data;

    for (size_t i = 0; i < subset->count; i++) {
        uint64_t id = sub_ids[i];
        bool found = false;
        for (size_t j = 0; j < set->count; j++) {
            if (set_ids[j] == id) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

#include <inttypes.h>

static inline void ecs_vec_print_type(const ecs_vec *type) {
    const uint64_t *ids = (const uint64_t*)type->data;

    printf("(");
    for (size_t i = 0; i < type->count; i++) {
        printf("%" PRIu64, ids[i]);
        if (i + 1 < type->count) {
            printf(", ");
        }
    }
    printf(")\n");
    puts("");
}



#endif
