#ifndef ECS_ARCHETYPE_H
    #define ECS_ARCHETYPE_H
    #include "ecs_entity.hpp"
    #include "ecs_type.hpp"
    #include "sparseset.hpp"
    #include "vec.h"
    #include "rayflect.hpp"
    #include <bitset>
    #include <cstddef>

namespace ecs {
    constexpr size_t MaxComponents = 128;
    using ArchetypeMask = std::bitset<MaxComponents>;

    class Archetype {
        SparseSet<ecs_vec> component_vec;
        ecs_vec entity_row = ecs_vec_create(sizeof(EntityIndex));

        public:
            ArchetypeMask mask;

            template<typename ...Components>
            Archetype() {
                (registerComponent<Components>(), ...);
            }

            Archetype(ArchetypeMask mask) {
                this->mask = 0;
                for (ComponentID id = 0; id < MaxComponents; ++id) {
                    if (mask.test(id)) {
                        registerComponent(id);
                    }
                }
            }

            Archetype() {}

            void copyRow(ComponentID id, Archetype *dst, size_t row, size_t newRow) {
                ecs_vec_copy_element(&component_vec.get(id), &dst->component_vec.get(id), row, newRow);
            }

            EntityRowUpdate remove(EntityIndex idx, size_t row) {

                for (ecs_vec &vec : component_vec.values()) {
                    ecs_vec_remove_fast(&vec, row);
                }
                EntityRowUpdate update = {
                    .index = *ECS_VEC_GET(EntityIndex, &entity_row, entity_row.count - 1),
                    .newRow = row
                };
                ecs_vec_remove_fast(&entity_row, row);
                return update;
            }

            size_t add(Entity e) {
                for (ecs_vec &vec : component_vec.values()) {
                    ecs_vec_push_zero(&vec);
                }
                ecs_vec_push(&entity_row, &e.index);
                return entity_row.count - 1;
            }

            size_t size() {
                return entity_row.count;
            }


        template<typename Component>
        void setComponent(size_t row, Component *component) {
            ComponentID id = rayflect::getComponentID<Component>();
            ecs_vec &vec = component_vec.get(id);
            ecs_vec_set(&vec, row, component);
        }

        void setComponentPtr(size_t row, ComponentID id, void *component) {
            ecs_vec &vec = component_vec.get(id);
            ecs_vec_set(&vec, row, component);
        }

        template<typename Component>
        Component *getComponent(size_t row) {
            ComponentID id = rayflect::getComponentID<Component>();
            ecs_vec &vec = component_vec.get(id);
            return ECS_VEC_GET(Component, &vec, row);
        }

        template<typename Component>
        Component &getComponentRef(size_t row) {
            ComponentID id = rayflect::getComponentID<Component>();
            ecs_vec &vec = component_vec.get(id);
            return *ECS_VEC_GET(Component, &vec, row);
        }

        void *getComponentPtr(size_t row, ComponentID id) {
            ecs_vec &vec = component_vec.get(id);
            return ECS_VEC_GET(void, &vec, row);
        }

        void registerComponent(ComponentID id) {
            component_vec.set(id, ecs_vec_create(rayflect::getComponentSizeByID(id)));
            mask.set(id);
        }

        template<typename Component>
        void registerComponent() {
            component_vec.set(rayflect::getComponentID<Component>(), ecs_vec_create(sizeof(Component)));
        }
    };
}
#endif
