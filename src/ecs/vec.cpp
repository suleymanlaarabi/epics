#include "vec.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void ecs_vec_grow(ecs_vec* v) {
    size_t new_cap = v->capacity ? v->capacity * 3 : 4;
    v->data = realloc(v->data, new_cap * v->elem_size);
    assert(v->data && "Error realloc");
    v->capacity = new_cap;
}

ecs_vec ecs_vec_init(size_t elem_size, size_t capacity) {
    ecs_vec v;
    v.size = 0;
    v.capacity = capacity ? capacity : 4;
    v.elem_size = elem_size;
    v.data = malloc(v.capacity * elem_size);
    assert(v.data && "Error malloc");
    return v;
}

void ecs_vec_free(ecs_vec* v) {
    free(v->data);
    v->data = NULL;
    v->size = v->capacity = v->elem_size = 0;
}

void ecs_vec_push(ecs_vec* v, const void* elem) {
    if (v->size == v->capacity) {
        ecs_vec_grow(v);
    }
    memcpy((char*)v->data + v->size * v->elem_size, elem, v->elem_size);
    v->size++;
}

void ecs_vec_pop(ecs_vec* v) {
    if (v->size > 0) v->size--;
}

void* ecs_vec_get(ecs_vec* v, size_t index) {
    assert(index < v->size && "Index out limit");
    return (char*)v->data + index * v->elem_size;
}

void ecs_vec_set(ecs_vec* v, size_t index, const void* elem) {
    assert(index < v->size && "Index out limit");
    memcpy((char*)v->data + index * v->elem_size, elem, v->elem_size);
}

void *ecs_vec_emplace_back(ecs_vec* v) {
    if (v->size == v->capacity) {
        ecs_vec_grow(v);
    }
    void *elem = (char*)v->data + v->size * v->elem_size;
    v->size++;
    return elem;
}
