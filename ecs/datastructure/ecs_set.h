#ifndef ECS_SET_H
    #define ECS_SET_H

    #include "../include/ecs_config.h"
    #include <stddef.h>
    #include <stdint.h>
    #include <stdlib.h>
    #include <string.h>

typedef struct {
    uint32_t *keys;
    uint8_t *used;
    size_t capacity;
    size_t count;
} ecs_set;

static inline void ecs_set_init(ecs_set *s, size_t capacity) {
    s->keys = (uint32_t*) calloc(capacity, sizeof(uint32_t));
    s->used = (uint8_t*) calloc(capacity, 1);
    s->capacity = capacity;
    s->count = 0;
}

static inline void ecs_set_free(ecs_set *s) {
    free(s->keys);
    free(s->used);
    s->keys = NULL;
    s->used = NULL;
    s->capacity = 0;
    s->count = 0;
}

static inline size_t ecs__hash(uint32_t k) {
    return (size_t)(k * 2654435761u);
}

static inline int ecs_set_has(const ecs_set *s, uint32_t k) {
    size_t h = ecs__hash(k) & (s->capacity - 1);
    while (s->used[h]) {
        if (s->keys[h] == k) return 1;
        h = (h + 1) & (s->capacity - 1);
    }
    return 0;
}

static inline void ecs_set_add(ecs_set *s, uint32_t k) {
    size_t h = ecs__hash(k) & (s->capacity - 1);
    while (s->used[h]) {
        if (ECS_UNLIKELY(s->keys[h] == k)) return;
        h = (h + 1) & (s->capacity - 1);
    }
    s->keys[h] = k;
    s->used[h] = 1;
    s->count++;
}

#endif
