#ifndef ECS_VEC_SORT_H
    #define ECS_VEC_SORT_H

    #include <stdint.h>
    #include <stdlib.h>
    #include <string.h>
    #include "ecs_vec.h"

static inline void insertion_sort_uint64(uint64_t *arr, size_t n) {
    for (size_t i = 1; i < n; i++) {
        uint64_t key = arr[i];
        size_t j = i;
        while (j > 0 && arr[j - 1] > key) {
            arr[j] = arr[j - 1];
            j--;
        }
        arr[j] = key;
    }
}

static inline int cmp_uint64(const void *a, const void *b) {
    uint64_t ua = *(const uint64_t*)a;
    uint64_t ub = *(const uint64_t*)b;
    return (ua > ub) - (ua < ub);
}

static inline void ecs_vec_sort_u64(ecs_vec *v) {
    if (!v || v->size != sizeof(uint64_t) || v->count == 0)
        return;

    uint64_t *arr = (uint64_t*)v->data;
    size_t n = v->count;

    if (n <= 50) {
        insertion_sort_uint64(arr, n);
    } else {
        qsort(arr, n, sizeof(uint64_t), cmp_uint64);
    }
}

#endif
