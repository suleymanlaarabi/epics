#ifndef ECS_ARCHETYPE_HPP
    #define ECS_ARCHETYPE_HPP
    #include <cstdint>
    #include <unordered_map>
    #include <unordered_set>
    #include <vector>
    #include "ecs_fwd.hpp"
    #include "component.hpp"
    #include "vec.h"

namespace ecs {
    struct Archetype {
        EntityType type;
        std::unordered_set<ComponentID> type_set;
        std::vector<Entity> entities;
        std::unordered_map<ComponentID, ecs_vec> components_vec;
        std::unordered_map<Entity, uint64_t> entity_indices;

        template <typename... Components>
        static Archetype create() {
            Archetype arch;
            (arch.addComponent<Components>(), ...);
            return arch;
        }

        static Archetype create(EntityType type) {
            Archetype arch;
            arch.type = type;
            for (auto id : type.getComponents()) {
                arch.components_vec[id] = ecs_vec_init(getComponentSizeByID(id), 30);
                arch.type_set.insert(id);
            }
            return arch;
        }

        template<typename ...Components>
        static EntityType getEntityType() {
            EntityType type;
            type.addComponents<Components...>();
            return type;
        }


        void addEntity(Entity entity) {
            this->entities.push_back(entity);
            this->entity_indices[entity] = this->entities.size() - 1;
            for (auto id : this->type.getComponents()) {
                ecs_vec_emplace_back(&this->components_vec[id]);
            }
        }

        template<typename Component>
        Component* getComponentArray() {
            ComponentID id = getComponentID<Component>();
            return static_cast<Component*>(this->components_vec[id].data);
        }

        template<typename Component>
        void addComponent() {
            ComponentID id = getComponentID<Component>();
            this->type.addComponent(id);
            this->type_set.insert(id);
        }

        template<typename Component>
        Component* getComponent(Entity entity) {
            ComponentID id = getComponentID<Component>();
            Component* components = static_cast<Component*>(this->components_vec[id].data);
            uint64_t index = this->entity_indices[entity];
            return &components[index];
        }

        template<typename Component>
        void setComponent(Entity entity, Component&& component) {
            ComponentID id = getComponentID<Component>();
            Component* components = static_cast<Component*>(this->components_vec[id].data);
            uint64_t index = this->entity_indices[entity];
            components[index] = std::forward<Component>(component);
        }

    };
}

#endif
