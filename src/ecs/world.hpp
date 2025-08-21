#ifndef ECS_WORLD_HPP
    #define ECS_WORLD_HPP
    #include <unordered_map>
    #include "archetype.hpp"
#include "component.hpp"
    #include "ecs_fwd.hpp"

namespace ecs {
    struct World {
        private:
            Entity entity_count = 0;

        public:
            std::unordered_map<Entity, Archetype*> entity_index;
            std::unordered_map<EntityType, Archetype, EntityTypeHash, EntityTypeEq> archetype_index;

            World() {
                Archetype empty = Archetype::create<>();
                archetype_index.insert({empty.type, empty});
            }

            Entity entity() {
                this->entity_count += 1;
                entity_index[entity_count] = &getArchetype<>();
                return this->entity_count;
            }

            template<typename Component>
            void addComponent(Entity entity) {
                EntityType type = getEntityType(entity);
                type.addComponent<Component>();
                Archetype& archetype = getArchetype(type);
                entity_index[entity] = &archetype;
            }

            template<typename Component>
            void setComponent(Entity entity, Component&& component) {
                Archetype& archetype = getArchetype(entity);

                if (hasComponent<Component>(entity)) {
                    archetype.setComponent<Component>(entity, std::forward<Component>(component));
                } else {
                    addComponent<Component>(entity);
                    archetype = getArchetype(entity);
                    archetype.setComponent(entity, std::forward<Component>(component));
                }
            }

            template<typename Component>
            Component* getComponent(Entity entity) {
                EntityType type = getEntityType(entity);
                Archetype& archetype = getArchetype(type);
                return archetype.getComponent<Component>(entity);
            }

            template<typename Component>
            bool hasComponent(Entity e) {
                Archetype* arch = entity_index[e];
                return arch->type_set.count(getComponentID<Component>());
            }

            EntityType getEntityType(Entity e) {
                Archetype* arch = entity_index[e];
                return arch->type;
            }

            template<typename ...Components>
            Archetype &getArchetype() {
                EntityType type = Archetype::getEntityType<Components...>();
                auto it = archetype_index.find(type);
                if (it != archetype_index.end()) {
                    return it->second;
                } else {
                    auto res = archetype_index.insert({type, Archetype::create<Components...>()});
                    return res.first->second;
                }
            }

            Archetype &getArchetype(EntityType type) {
                auto it = archetype_index.find(type);
                if (it != archetype_index.end()) {
                    return it->second;
                } else {
                    auto res = archetype_index.insert({type, Archetype::create(type)});
                    return res.first->second;
                }
            }

            Archetype &getArchetype(Entity entity) {
                Archetype* arch = entity_index[entity];
                return *arch;
            }
    };
}

#endif
