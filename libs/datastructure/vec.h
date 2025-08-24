#ifndef VEC_H
    #define VEC_H

    #include <cstddef>
    #include <stddef.h>
    #include <stdlib.h>
    #include <string.h>
    #ifndef ECS_LIKELY
    #  if defined(__GNUC__) || defined(__clang__)
    #    define ECS_LIKELY(x)   __builtin_expect(!!(x), 1)
    #    define ECS_UNLIKELY(x) __builtin_expect(!!(x), 0)
    #  else
    #    define ECS_LIKELY(x)   (x)
    #    define ECS_UNLIKELY(x) (x)
    #  endif
    #endif

typedef struct {
    void *data;
    size_t count;
    size_t capacity;
    size_t size;
} ecs_vec;

static inline void ecs_vec_init(ecs_vec *v, size_t size) {
    v->data = NULL;
    v->count = 0;
    v->capacity = 0;
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

static inline void ecs_vec_copy(ecs_vec *src, ecs_vec *dest) {
    if (src->size != dest->size) {
        ecs_vec_free(dest);
        ecs_vec_init(dest, src->size);
    }
    dest->count = src->count;
    dest->capacity = src->capacity;
    ecs_vec_ensure(dest, src->count);
    memcpy(dest->data, src->data, src->count * src->size);
}

static inline void ecs_vec_set(ecs_vec *v, size_t index, const void *elem) {
    if (ECS_UNLIKELY(index >= v->count)) {
        ecs_vec_ensure(v, index + 1);
        v->count = index + 1;
    }
    memcpy((char *)v->data + index * v->size, elem, v->size);
}

// static inline void ecs_vec_remove(ecs_vec *v, size_t index) {
//     if (ECS_UNLIKELY(index >= v->count)) {
//         return;
//     }
//     size_t size = v->size;
//     char *data = (char *)v->data;
//     memmove(data + index * size, data + (index + 1) * size, (v->count - index - 1) * size);
//     v->count--;
// }

static inline void ecs_vec_remove_fast(ecs_vec *v, size_t index) {
    if (ECS_UNLIKELY(index >= v->count)) {
        return;
    }
    size_t size = v->size;
    char *data = (char *)v->data;
    memcpy(data + index * size, data + (v->count - 1) * size, size);
    v->count--;
}

#define ECS_VEC_GET(type, v, i) ((type *)((char *)(v)->data + (i) * (v)->size))

#endif
