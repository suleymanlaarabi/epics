#ifndef ECS_VEC_HASHMAP_H
    #define ECS_VEC_HASHMAP_H
    #include <stddef.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdint.h>
    #include <string.h>
    #include <stdbool.h>
    #include "ecs_vec.h"

static inline uint64_t ecs_vec_hash(const ecs_vec *v) {
    const uint8_t *bytes = (const uint8_t*)v->data;
    uint64_t hash = 1469598103934665603ULL;
    size_t total = v->count * v->size;
    for (size_t i = 0; i < total; i++) {
        hash ^= bytes[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

static inline bool ecs_vec_equal(const ecs_vec *a, const ecs_vec *b) {
    if (a->count != b->count) return false;
    if (a->size != b->size) return false;
    size_t total = a->count * a->size;
    if (total == 0) return true;
    return memcmp(a->data, b->data, total) == 0;
}

typedef enum { ECS_BUCKET_EMPTY = 0, ECS_BUCKET_OCCUPIED = 1, ECS_BUCKET_DELETED = 2 } ecs_bucket_state;

typedef struct {
    uint64_t hash;
    ecs_vec key;
    void *value;
    ecs_bucket_state state;
} ecs_bucket;

typedef struct {
    size_t elem_size;
    size_t cap;
    size_t len;
    ecs_bucket *buckets;
} ecs_hashmap;

static inline size_t ecs_round_up_power_of_two(size_t v) {
    if (v == 0) return 1;
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
#if SIZE_MAX > 0xFFFFFFFF
    v |= v >> 32;
#endif
    return v + 1;
}

static inline void ecs_bucket_init_empty(ecs_bucket *b) {
    b->hash = 0;
    b->key.data = NULL;
    b->key.count = 0;
    b->key.size = 0;
    b->value = NULL;
    b->state = ECS_BUCKET_EMPTY;
}

static inline ecs_hashmap ecs_hashmap_new(size_t elem_size, size_t initial_cap) {
    ecs_hashmap m;
    if (initial_cap < 8) initial_cap = 8;
    initial_cap = ecs_round_up_power_of_two(initial_cap);
    m.elem_size = elem_size;
    m.cap = initial_cap;
    m.len = 0;
    m.buckets = (ecs_bucket*)malloc(sizeof(ecs_bucket) * m.cap);
    for (size_t i = 0; i < m.cap; ++i) ecs_bucket_init_empty(&m.buckets[i]);
    return m;
}

static inline void ecs_hashmap_free(ecs_hashmap *m) {
    if (m->buckets) {
        for (size_t i = 0; i < m->cap; ++i) {
            ecs_bucket *b = &m->buckets[i];
            if (b->state == ECS_BUCKET_OCCUPIED) {
                if (b->key.data) free(b->key.data);
                if (b->value) free(b->value);
            }
        }
        free(m->buckets);
        m->buckets = NULL;
    }
    m->cap = 0;
    m->len = 0;
    m->elem_size = 0;
}

static inline void ecs_hashmap_reinsert_bucket(ecs_bucket *dst_buckets, size_t dst_cap, ecs_bucket *src_b) {
    size_t idx = src_b->hash & (dst_cap - 1);
    while (true) {
        ecs_bucket *d = &dst_buckets[idx];
        if (d->state == ECS_BUCKET_EMPTY) {
            d->hash = src_b->hash;
            d->key = src_b->key;
            d->value = src_b->value;
            d->state = ECS_BUCKET_OCCUPIED;
            src_b->key.data = NULL;
            src_b->value = NULL;
            src_b->state = ECS_BUCKET_EMPTY;
            return;
        }
        idx = (idx + 1) & (dst_cap - 1);
    }
}

static inline void ecs_hashmap_resize(ecs_hashmap *m, size_t new_cap) {
    if (!m) return;
    if (new_cap < 8) new_cap = 8;
    new_cap = ecs_round_up_power_of_two(new_cap);
    ecs_bucket *new_buckets = (ecs_bucket*)malloc(sizeof(ecs_bucket) * new_cap);

    for (size_t i = 0; i < new_cap; ++i) ecs_bucket_init_empty(&new_buckets[i]);

    for (size_t i = 0; i < m->cap; ++i) {
        ecs_bucket *b = &m->buckets[i];
        if (b->state == ECS_BUCKET_OCCUPIED) {
            ecs_hashmap_reinsert_bucket(new_buckets, new_cap, b);
        } else {
            if (b->key.data) { free(b->key.data); b->key.data = NULL; }
            if (b->value) { free(b->value); b->value = NULL; }
        }
    }
    free(m->buckets);
    m->buckets = new_buckets;
    m->cap = new_cap;
}

static inline size_t ecs_hashmap_find_slot(const ecs_hashmap *m, const ecs_vec *key, uint64_t h, bool *found_out) {
    size_t cap = m->cap;
    size_t idx = h & (cap - 1);
    ssize_t first_deleted = -1;
    for (;;) {
        const ecs_bucket *b = &m->buckets[idx];
        if (b->state == ECS_BUCKET_EMPTY) {
            if (first_deleted != -1) {
                *found_out = false;
                return (size_t)first_deleted;
            } else {
                *found_out = false;
                return idx;
            }
        } else if (b->state == ECS_BUCKET_DELETED) {
            if (first_deleted == -1) first_deleted = (ssize_t)idx;
        } else {
            if (b->hash == h && ecs_vec_equal(&b->key, key)) {
                *found_out = true;
                return idx;
            }
        }
        idx = (idx + 1) & (cap - 1);
    }
}


static inline void *ecs_hashmap_ensure(ecs_hashmap *m, const ecs_vec *key) {
    if (m->len * 100 >= m->cap * 70) {
        ecs_hashmap_resize(m, m->cap * 2);
    }
    uint64_t h = ecs_vec_hash(key);
    bool found = false;
    size_t idx = ecs_hashmap_find_slot(m, key, h, &found);
    ecs_bucket *b = &m->buckets[idx];
    if (found) {
        if (!b->value) {
            b->value = malloc(m->elem_size);
            memset(b->value, 0, m->elem_size);
        }
        return b->value;
    }

    b->hash = h;
    b->state = ECS_BUCKET_OCCUPIED;

    b->key.count = key->count;
    b->key.size = key->size;
    size_t total = key->count * key->size;
    if (total) {
        b->key.data = malloc(total);
        memcpy(b->key.data, key->data, total);
    } else {
        b->key.data = NULL;
    }
    b->value = malloc(m->elem_size);
    memset(b->value, 0, m->elem_size);
    m->len++;
    return b->value;
}

static inline void ecs_hashmap_put(ecs_hashmap *m, const ecs_vec *key, const void *value) {
    if (!m) return;
    void *dst = ecs_hashmap_ensure(m, key);
    if (dst && value) memcpy(dst, value, m->elem_size);
}

static inline bool ecs_hashmap_get(const ecs_hashmap *m, const ecs_vec *key, void *out) {
    if (!m) return false;
    uint64_t h = ecs_vec_hash(key);
    bool found = false;
    size_t idx = ecs_hashmap_find_slot(m, key, h, &found);
    if (!found) return false;
    const ecs_bucket *b = &m->buckets[idx];
    if (!b->value) return false;
    if (out) memcpy(out, b->value, m->elem_size);
    return true;
}

static inline bool ecs_hashmap_get_ptr(const ecs_hashmap *m, const ecs_vec *key, void **out) {
    if (!m || !out) return false;
    uint64_t h = ecs_vec_hash(key);
    bool found = false;
    size_t idx = ecs_hashmap_find_slot(m, key, h, &found);
    if (!found) return false;
    const ecs_bucket *b = &m->buckets[idx];
    if (!b->value) return false;
    *out = b->value;
    return true;
}

static inline bool ecs_hashmap_remove(ecs_hashmap *m, const ecs_vec *key) {
    if (!m) return false;
    uint64_t h = ecs_vec_hash(key);
    bool found = false;
    size_t idx = ecs_hashmap_find_slot(m, key, h, &found);
    if (!found) return false;
    ecs_bucket *b = &m->buckets[idx];
    if (b->state != ECS_BUCKET_OCCUPIED) return false;
    if (b->key.data) { free(b->key.data); b->key.data = NULL; }
    if (b->value) { free(b->value); b->value = NULL; }
    b->state = ECS_BUCKET_DELETED;
    m->len--;
    return true;
}

typedef struct {
    const ecs_vec *key;
    void *value;
} ecs_hashmap_entry;

static inline bool ecs_hashmap_next(const ecs_hashmap *m, size_t *i, ecs_hashmap_entry *out) {
    if (!m || !i || !out) return false;
    size_t idx = *i;
    while (idx < m->cap) {
        const ecs_bucket *b = &m->buckets[idx];
        idx++;
        if (b->state == ECS_BUCKET_OCCUPIED) {
            out->key = &b->key;
            out->value = b->value;
            *i = idx;
            return true;
        }
    }
    *i = idx;
    return false;
}

#endif
