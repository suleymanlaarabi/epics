#ifndef ECS_VEC_H
    #define ECS_VEC_H

    #include <stddef.h>
    #include <stddef.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "../ecs/ecs_utils.h"


typedef struct {
    void *data;
    size_t count;
    size_t capacity;
    size_t size;
} ecs_vec;

static inline void ecs_vec_init(ecs_vec *v, size_t size) {
    v->data = malloc(16 * size);
    v->count = 0;
    v->capacity = 16;
    v->size = size;
}

static inline ecs_vec ecs_vec_create(size_t size) {
    ecs_vec v;
    ecs_vec_init(&v, size);
    return v;
}

static inline void ecs_vec_free(ecs_vec *v) {
    free(v->data);
    v->data = NULL;
    v->count = 0;
    v->capacity = 0;
    v->size = 0;
}

static inline void *ecs_vec_push(ecs_vec *v, const void *elem) {
    if (ECS_UNLIKELY(v->count >= v->capacity)) {
        size_t new_capacity = v->capacity ? v->capacity * 3 : 16;
        v->data = realloc(v->data, new_capacity * v->size);
        v->capacity = new_capacity;
    }
    void *dest = (char *)v->data + v->count * v->size;
    memcpy(dest, elem, v->size);
    v->count++;
    return dest;
}

static inline void ecs_vec_push_zero(ecs_vec *v) {
    if (ECS_UNLIKELY(v->count >= v->capacity)) {
        size_t new_capacity = v->capacity ? v->capacity * 3 : 16;
        v->data = realloc(v->data, new_capacity * v->size);
        v->capacity = new_capacity;
    }
    void *dest = (char *)v->data + v->count * v->size;
    memset(dest, 0, v->size);
    v->count++;
}

static inline void ecs_vec_copy_element(ecs_vec *src, ecs_vec *dest, size_t index, size_t dest_index) {
    void *src_ptr = (char *)src->data + index * src->size;
    void *dest_ptr = (char *)dest->data + dest_index * dest->size;
    memcpy(dest_ptr, src_ptr, src->size);
}

static inline void ecs_vec_ensure(ecs_vec *v, size_t count) {
    if (ECS_UNLIKELY(count > v->capacity)) {
        size_t new_capacity = count;
        v->data = realloc(v->data, new_capacity * v->size);
        v->capacity = new_capacity;
    }
}

static inline void *ecs_vec_add(ecs_vec *v) {
    ecs_vec_push_zero(v);
    return (char *)v->data + (v->count - 1) * v->size;
}

static inline void ecs_vec_copy(ecs_vec *src, ecs_vec *dest) {
    ecs_vec_init(dest, src->size);
    ecs_vec_ensure(dest, src->count + 1);
    dest->count = src->count;
    dest->capacity = src->capacity;
    memcpy(dest->data, src->data, src->count * src->size);
}

static inline void ecs_vec_set(ecs_vec *v, size_t index, const void *elem) {
    if (ECS_UNLIKELY(index >= v->count)) {
        ecs_vec_ensure(v, index + 1);
        v->count = index + 1;
    }
    memcpy((char *)v->data + index * v->size, elem, v->size);
}

static inline void ecs_vec_remove_fast(ecs_vec *v, size_t index) {
    if (ECS_UNLIKELY(index >= v->count)) {
        return;
    }
    size_t size = v->size;
    char *data = (char *)v->data;
    memcpy(data + index * size, data + (v->count - 1) * size, size);
    v->count--;
}

static inline ecs_vec ecs_vec_concat(ecs_vec *v1, ecs_vec *v2) {
    ecs_vec result = {0};
    ecs_vec_init(&result, v1->size);
    ecs_vec_ensure(&result, v1->count + v2->count);
    memcpy(result.data, v1->data, v1->count * v1->size);
    memcpy((char *)result.data + v1->count * v1->size, v2->data, v2->count * v2->size);
    result.count = v1->count + v2->count;
    result.size = v1->size;
    return result;
}

static inline void ecs_vec_remove_ordered(ecs_vec *v, size_t index) {
    if (ECS_UNLIKELY(index >= v->count)) {
        return;
    }
    size_t size = v->size;
    char *data = (char *)v->data;
    memmove(data + index * size, data + (index + 1) * size, (v->count - index - 1) * size);
    v->count--;
}

static inline void ecs_vec_remove_last(ecs_vec *v) {
    if (ECS_UNLIKELY(v->count == 0)) {
        return;
    }
    v->count--;
}

static inline void debug_vec(ecs_vec *v) {
    printf("Vector capacity: %zu\n", v->capacity);
    printf("Vector count: %zu\n", v->count);
    printf("Vector size: %zu\n", v->size);
    printf("dataptr: %p\n", v->data);
}

#define ECS_VEC_GET(type, v, i) ((type *)((char *)(v)->data + (i) * (v)->size))
#define ECS_VEC_GET_LAST(type, v) ((type *)((char *)(v)->data + ((v)->count - 1) * (v)->size))
#define ECS_RAW_VEC(type, ...) {                                       \
    .capacity = sizeof((type[]){__VA_ARGS__}) / sizeof(type),          \
    .count    = sizeof((type[]){__VA_ARGS__}) / sizeof(type),          \
    .data     = (type[]){__VA_ARGS__},                                 \
    .size     = sizeof(type)                                           \
}
#endif
