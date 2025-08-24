#ifndef SPARSESET_H
    #define SPARSESET_H

    #include "vec.h"
    #include <cstddef>
    #include <cstdint>
    #include "span.hpp"

    using u32 = uint32_t;

struct SparseSetDenseRecord {
    u32 index;
    bool is_valid;
};

template<typename Component>
class SparseSet {
    ecs_vec sparse = ecs_vec_create(sizeof(SparseSetDenseRecord));
    ecs_vec dense = ecs_vec_create(sizeof(Component));

    public:
        inline void set(u32 id, Component value) {
            if (has(id)) {
                Component& existing_value = get(id);
                existing_value = value;
                return;
            }

            SparseSetDenseRecord record = {
                .index = (u32) dense.count,
                .is_valid = true
            };
            ecs_vec_push(&dense, &value);
            ecs_vec_set(&sparse, id, &record);
        }

        inline Component& get(u32 id) {
            return *ECS_VEC_GET(Component, &dense, ECS_VEC_GET(SparseSetDenseRecord, &sparse, id)->index);
        }

        inline Component& getOrAdd(u32 id) {
            if (has(id)) {
                return get(id);
            }
            Component value{};
            set(id, value);
            return get(id);
        }

        inline bool has(u32 id) {
            if (id >= sparse.count) return false;
            return ECS_VEC_GET(SparseSetDenseRecord, &sparse, id)->is_valid;
        }

        inline Span<Component> values() {
            return { static_cast<Component*>(dense.data), (u32)dense.count };
        }

        inline size_t len() const {
            return dense.count;
        }
};

#endif
