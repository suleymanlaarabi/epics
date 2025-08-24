#ifndef ECS_WORLD_H
    #define ECS_WORLD_H
    #include "ecs_archetype.hpp"
    #include "ecs_entity.hpp"
    #include "ecs_type.hpp"
    #include "hash.hpp"
    #include "rayflect.hpp"
    #include <bitset>
    #include <cstddef>
    #include <cstring>
    #include <unordered_map>
    #include <vector>

namespace ecs {

    struct EntityRecord {
        Archetype *archetype;
        ArchetypeMask mask;
        size_t row;
    };

    struct System {
        ArchetypeMask signature;
        std::function<void(Archetype&)> invoker;
        std::vector<Archetype*> matches;

        void addArchetype(Archetype *archetype) {
            matches.push_back(archetype);
        }

        void invoke() {
            for (Archetype* archetype : matches) {
                invoker(*archetype);
            }
        }
    };

    class World {
        private:
            EntityManager entityManager;

            std::vector<EntityRecord> entity_record;

            std::unordered_map<ArchetypeMask, Archetype, MaskHash> archetypes;

            std::vector<System> systems;


            void setArchetype(Entity e, ArchetypeMask mask) {
                EntityRecord& record = entity_record[e.index];
                record.archetype = &archetypes[mask];
            }

            void addArchetype(ArchetypeMask mask) {
                archetypes[mask] = Archetype(mask);

                for (auto& system : systems) {
                    if ((system.signature & mask) == system.signature) {
                        system.matches.push_back(&archetypes[mask]);
                    }
                }
            }

            Archetype* getArchetypePtr(ArchetypeMask mask) {
                if (!archetypes.count(mask)) {
                    addArchetype(mask);
                }
                return &archetypes[mask];
            }

            template<typename ...Components>
            ArchetypeMask archetypeMask() {
                ArchetypeMask mask;

                (mask.set(rayflect::getComponentID<Components>()), ...);
                return mask;
            }

            void updateEntityRecordAfterMaskChange(Entity e) {
                EntityRecord &record = entity_record[e.index];
                Archetype *oldArchetype = record.archetype;

                Archetype *newArchetype = getArchetypePtr(record.mask);
                size_t newRow = newArchetype->add(e);

                copyArchetypeRow(oldArchetype, newArchetype, record.row, newRow);

                EntityRowUpdate update = oldArchetype->remove(e.index, record.row);
                entity_record[update.index].row = update.newRow;

                record.archetype = newArchetype;
                record.row = newRow;
            }

            void copyArchetypeRow(Archetype *src, Archetype *dst, size_t row, size_t newRow) {
                struct Position {
                    float x;
                    float y;
                };
                for (size_t i = 0; i < MaxComponents; i++) {
                    if (src->mask.test(i) && dst->mask.test(i)) {
                        src->copyRow(i, dst, row, newRow);
                    }
                }
            }


        public:
            Entity entity() {
                Entity e = entityManager.entity();
                Archetype *archetype = getArchetypePtr(0);
                size_t row = archetype->add(e);

                if (e.index >= entity_record.size()) {
                    entity_record.push_back({
                        .archetype = archetype,
                        .row = row,
                        .mask = archetype->mask
                    });
                    return e;
                }
                entity_record[e.index].archetype = archetype;
                entity_record[e.index].row = row;
                return e;
            }

            template <typename... Components, typename Func>
            void system(Func f) {
                static_assert(std::is_invocable_v<Func, Components&...>,
                              "Func must be callable with Components&...");

                ArchetypeMask mask = archetypeMask<Components...>();
                System sys = {
                    .signature = mask,
                    .invoker = [f](Archetype& archetype) {
                        for (size_t row = 0; row < archetype.size(); ++row) {
                            f(archetype.getComponentRef<Components>(row)...);
                        }
                    },
                };
                for (auto& [mask, archetype] : archetypes) {
                    if ((sys.signature & mask) == sys.signature) {
                        sys.matches.push_back(&archetype);
                    }
                }
                systems.push_back(sys);
            }

            void kill(Entity e) {
                EntityRecord &record = entity_record[e.index];
                record.archetype->remove(e.index, record.row);
                entityManager.remove(e);
            }

            void progress() {
                for (System &sys : systems) {
                    sys.invoke();
                }
            }

            bool isAlive(Entity e) {
                return entityManager.isAlive(e);
            }

            template<typename Component>
            void add(Entity e) {
                EntityRecord &record = entity_record[e.index];
                entity_record[e.index].mask.set(rayflect::getComponentID<Component>());
                updateEntityRecordAfterMaskChange(e);
            }

            template<typename Component>
            void add(Entity e, const Component &component) {
                ComponentID componentID = rayflect::getComponentID<Component>();
                entity_record[e.index].mask.set(componentID);
                updateEntityRecordAfterMaskChange(e);
                entity_record[e.index].archetype->setComponentPtr(entity_record[e.index].row, componentID, (Component *) &component);
            }

            template<typename Component>
            Component& get(Entity e) {
                EntityRecord& record = entity_record[e.index];
                return *record.archetype->getComponent<Component>(record.row);
            }

            template<typename Component>
            void remove(Entity e) {
                EntityRecord &record = entity_record[e.index];
                record.mask.reset(rayflect::getComponentID<Component>());
                updateEntityRecordAfterMaskChange(e);
            }

            template<typename Component>
            bool has(Entity e) {
                EntityRecord& record = entity_record[e.index];
                return record.mask.test(rayflect::getComponentID<Component>());
            }

    };
}

#endif
