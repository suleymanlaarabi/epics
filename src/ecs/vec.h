#ifndef VEC_H
    #define VEC_H
    #include <stddef.h>

typedef struct {
    void* data;
    size_t size;
    size_t capacity;
    size_t elem_size;
} ecs_vec;

ecs_vec ecs_vec_init(size_t elem_size, size_t capacity);

void ecs_vec_free(ecs_vec* v);

void ecs_vec_push(ecs_vec* v, const void* elem);

void ecs_vec_pop(ecs_vec* v);

void* ecs_vec_get(ecs_vec* v, size_t index);

void ecs_vec_set(ecs_vec* v, size_t index, const void* elem);

void *ecs_vec_emplace_back(ecs_vec* v);

#endif
