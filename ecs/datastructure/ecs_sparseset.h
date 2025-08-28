#ifndef ECS_SPARSESET_H
    #define ECS_SPARSESET_H
    #include "ecs_vec.h"
    #include <stdbool.h>
    #include <stdint.h>
    #include <stdio.h>
    #include <string.h>
    #define ECS_SPARSESET_GET_DENSE(set, entity) \
        ECS_VEC_GET(void, &(set)->dense, *ECS_SPARSESET_GET_SPARSE(set, entity))

    #define ECS_SPARSESET_GET_SPARSE(set, entity) \
        ECS_VEC_GET(uint64_t, &(set)->sparse, entity)

typedef struct {
    ecs_vec dense;
    ecs_vec sparse;
} ecs_sparseset;

static inline void ecs_sparseset_init(ecs_sparseset *sparse, size_t elem_size) {
    ecs_vec_init(&sparse->dense, elem_size);
    ecs_vec_init(&sparse->sparse, sizeof(uint64_t));
    memset(sparse->sparse.data, 0xFF, sparse->sparse.capacity * sizeof(uint64_t));
}

static inline void ecs_sparseset_free(ecs_sparseset *set) {
    ecs_vec_free(&set->dense);
    ecs_vec_free(&set->sparse);
}

static inline bool ecs_sparseset_contains(ecs_sparseset *set, uint64_t entity) {
    if (entity >= set->sparse.count) return false;
    return *ECS_VEC_GET(uint64_t, &set->sparse, entity) != UINT64_MAX;
}

static inline void *ecs_sparseset_get(ecs_sparseset *set, uint64_t entity) {

    if (ECS_UNLIKELY(!ecs_sparseset_contains(set, entity)))
        return NULL;

    return ECS_SPARSESET_GET_DENSE(set, entity);
}

static inline void ecs_sparseset_add(ecs_sparseset *set, uint64_t entity, void *data) {
    if (entity < set->sparse.count && ecs_sparseset_contains(set, entity)) {
        memcpy(ECS_SPARSESET_GET_DENSE(set, entity), data, set->dense.size);
        return;
    }

    ecs_vec_ensure(&set->sparse, entity + 1);

    if (set->sparse.count < entity + 1) {
        set->sparse.count = entity + 1;
    }

    *ECS_VEC_GET(uint64_t, &set->sparse, entity) = set->dense.count;
    ecs_vec_push(&set->dense, data);
}

static inline void ecs_sparseset_remove(ecs_sparseset *set, uint64_t entity) {
    if (ECS_UNLIKELY(!ecs_sparseset_contains(set, entity)))
        return;
    uint64_t index = *ECS_SPARSESET_GET_SPARSE(set, entity);
    *ECS_SPARSESET_GET_SPARSE(set, entity) = UINT64_MAX;
    if (index != set->dense.count - 1) {
        uint64_t moved_entity = 0;

        memcpy(&moved_entity, ECS_VEC_GET(void, &set->dense, set->dense.count - 1), sizeof(uint64_t));
        ecs_vec_remove_fast(&set->dense, index);
        *ECS_SPARSESET_GET_SPARSE(set, moved_entity) = index;
    }

}


#endif
