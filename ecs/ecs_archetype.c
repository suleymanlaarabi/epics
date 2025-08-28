#include "ecs_archetype.h"
#include "datastructure/ecs_sparseset.h"
#include "datastructure/ecs_vec.h"
#include "datastructure/ecs_vec_sort.h"
#include "ecs_types.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>


void ecs_archetype_init(ecs_archetype *archetype)
{
    ecs_sparseset_init(&archetype->cols, sizeof(ecs_vec));
    ecs_sparseset_init(&archetype->add_edge, sizeof(ecs_archetype *));
    ecs_sparseset_init(&archetype->remove_edge, sizeof(ecs_archetype *));
    ecs_vec_init(&archetype->type, sizeof(ecs_entity));
    ecs_vec_init(&archetype->entities, sizeof(uint32_t));
}

void ecs_archetype_add_col(ecs_archetype *archetype, ecs_entity component, size_t size)
{
    ecs_vec col;
    ecs_vec_init(&col, size);
    ecs_sparseset_add(&archetype->cols, component.value, &col);
    ecs_vec_push(&archetype->type, &component.value);
    ecs_vec_sort_u64(&archetype->type);
}

void ecs_archetype_add_singleton(ecs_archetype *archetype, ecs_entity component)
{
    ecs_vec col;
    ecs_vec_init(&col, 0);
    ecs_sparseset_add(&archetype->cols, component.value, &col);
    ecs_vec_push(&archetype->type, &component.value);
    ecs_vec_sort_u64(&archetype->type);
}

uint32_t ecs_archetype_add_entity(ecs_archetype *archetype, ecs_entity entity)
{
    ecs_vec *cols = archetype->cols.dense.data;
    size_t cols_len = archetype->cols.dense.count;

    for (size_t i = 0; i < cols_len; i++) {
        ecs_vec_push_zero(&cols[i]);
    }
    ecs_vec_push(&archetype->entities, &entity.index);
    return 0;
}

ecs_archetype_remove_result ecs_archetype_remove_entity(ecs_archetype *archetype, size_t row)
{
    ecs_archetype_remove_result result = {0};
    uint32_t *entities_data = (uint32_t *)archetype->entities.data;
    result.removed_entity_index = entities_data[row];

    if (row == archetype->entities.count - 1) {
        result.swapped_entity_new_row = UINT32_MAX;
    } else {
        result.swapped_entity_new_row = row;
    }
    ecs_vec *cols = (ecs_vec *)archetype->cols.dense.data;
    size_t cols_count = archetype->cols.dense.count;

    for (size_t i = 0; i < cols_count; i++) {
        ecs_vec_remove_fast(&cols[i], row);
    }
    ecs_vec_remove_fast(&archetype->entities, row);
    return result;
}
