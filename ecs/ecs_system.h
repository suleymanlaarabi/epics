#ifndef  ECS_SYSTEM_H
    #define ECS_SYSTEM_H
    #include "ecs_archetype.h"
    #include "ecs_types.h"
    #include "ecs_vec.h"
    #include <stddef.h>
    #include <stdint.h>


typedef struct {
    ecs_archetype *archetype;
    uint32_t count;
} ecs_iter;

typedef void (*ecs_system_func) (ecs_iter *it);

typedef struct {
    ecs_type query;
    ecs_type query_mut;
    ecs_type type;
    ecs_system_func func;
    ecs_vec matches;
} ecs_system;

typedef ecs_vec ecs_system_batch;

static inline uint64_t ecs_vec_get_u64(const ecs_vec *v, size_t i) {
    return *(uint64_t *)((char *)v->data + i * v->size);
}

static inline bool ecs_systems_are_compatible(
    const ecs_vec *q1,
    const ecs_vec *qm1,
    const ecs_vec *q2,
    const ecs_vec *qm2
) {
    for (size_t i = 0; i < q1->count; i++) {
        uint64_t id1 = ecs_vec_get_u64(q1, i);
        for (size_t j = 0; j < qm2->count; j++) {
            uint64_t id2 = ecs_vec_get_u64(qm2, j);
            if (id1 == id2) return false;
        }
    }

    for (size_t i = 0; i < q2->count; i++) {
        uint64_t id1 = ecs_vec_get_u64(q2, i);
        for (size_t j = 0; j < qm1->count; j++) {
            uint64_t id2 = ecs_vec_get_u64(qm1, j);
            if (id1 == id2) return false;
        }
    }

    for (size_t i = 0; i < qm1->count; i++) {
        uint64_t id1 = ecs_vec_get_u64(qm1, i);
        for (size_t j = 0; j < qm2->count; j++) {
            uint64_t id2 = ecs_vec_get_u64(qm2, j);
            if (id1 == id2) return false;
        }
    }

    return true;
}

#endif
