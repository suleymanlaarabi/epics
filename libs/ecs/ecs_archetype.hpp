#pragma once

#include "ecs_type.hpp"
#include "ecs_vecpp.hpp"
#include "sparseset.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <vector>

namespace ecs {

    class World;

    struct Archetype {
        Type type;
        SparseSet<EcsVec> cols;
        std::vector<u32> entities_indices;
        SparseSet<ArchetypeID> add_edge;
        SparseSet<ArchetypeID> remove_edge;

        Archetype(const Type& type, SparseSet<ComponentRecord> *components) {
            for (auto& component : type.data) {
                if (components->has(component.id())) {
                    register_component(component, components->get(component.id()).size);
                } else {
                    register_component(component, 0);
                }
            }
        }

        void print() {
            std::cout << "Entity Count: " << entities_indices.size() << std::endl;
        }

        inline void register_component(Entity entity, size_t size) {
            type.addComponent(entity);
            cols.set(entity.id(), EcsVec(size));
        }

        inline bool has_component(Entity entity) {
            return cols.has(entity.id());
        }

        inline void *getRawComponent(size_t row, Entity component) {
            return cols.get(component.id()).get(row);
        }

        inline void writeRawComponent(size_t row, Entity component, void *data) {
            cols.get(component.id()).set(row, data);
        }

        inline u64 addEntity(Entity entity) {
            for (auto& col : cols.values()) {
                col.pushZero();
            }
            entities_indices.push_back(entity.index);
            return entities_indices.size() - 1;
        }

        inline EntityUpdate removeEntity(u32 row) {
            u32 entity_index = entities_indices[row];
            EntityUpdate update{entity_index, row};
            for (auto& col : cols.values()) {
                col.removeFast(entity_index);
            }
            entities_indices.erase(entities_indices.begin() + row);
            return update;
            return {0};
        }

        inline ArchetypeID getAddEdge(Entity addComponent) {
            if (!add_edge.has(addComponent.id())) {
                return UINT64_MAX;
            }
            return add_edge.get(addComponent.id());
        }

        inline void setAddEdge(Entity addComponent, ArchetypeID newArchetypeID) {
            add_edge.set(addComponent.id(), newArchetypeID);
        }

        inline ArchetypeID getRemoveEdge(Entity removeComponent) {
            if (!remove_edge.has(removeComponent.id())) {
                return UINT64_MAX;
            }
            return remove_edge.get(removeComponent.id());
        }

        inline void setRemoveEdge(Entity removeComponent, ArchetypeID newArchetypeID) {
            remove_edge.set(removeComponent.id(), newArchetypeID);
        }

        inline u32 entityCount() const {
            return entities_indices.size();
        }
    };
}
