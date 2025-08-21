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
                Archetype* archetype = &getArchetype<>();
                entity_index[entity_count] = archetype;
                archetype->addEntity(entity_count);
                return entity_count;
            }

            template<typename Component>
            void addComponent(Entity entity) {

                if (hasComponent<Component>(entity)) {
                    return;
                } else {
                    Archetype &archetype = getArchetype(entity);
                    EntityType type = archetype.type;
                    type.addComponents<Component>();
                    Archetype &newArchetype = getArchetype(type);
                    newArchetype.addEntity(entity);
                    entity_index[entity] = &newArchetype;
                }
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
            void removeComponent(Entity entity) {
                Archetype& old_archetype = getArchetype(entity);
                old_archetype.removeEntity(entity);

                EntityType type = old_archetype.type;
                type.removeComponent(getComponentID<Component>());
                Archetype& archetype = getArchetype(type);
                entity_index[entity] = &archetype;
                archetype.addEntity(entity);
            }

            template<typename Component>
            Component* getComponent(Entity entity) {
                Archetype& archetype = getArchetype(entity);
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
