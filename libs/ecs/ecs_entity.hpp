#ifndef ECS_ENTITY_H
    #define ECS_ENTITY_H
    #include <cstddef>
    #include <cstdint>
    #include <vector>

namespace ecs {
    using EntityIndex = uint32_t;
    using EntityGeneration = uint32_t;

    struct Entity {
        EntityIndex index;
        EntityGeneration gen;
    };

    struct EntityRowUpdate {
        EntityIndex index;
        size_t newRow;
    };

    using OldEntity = Entity;

    class EntityManager {
        std::vector<EntityGeneration> generations;
        std::vector<EntityIndex> availableSlots;

        public:
            Entity entity() {
                if (availableSlots.empty()) {
                    Entity entity = {
                        .index = (uint32_t) generations.size(),
                        .gen = 0
                    };
                    generations.push_back(0);
                    return entity;
                }
                EntityIndex index = availableSlots.back();
                availableSlots.pop_back();
                EntityGeneration newGen = generations[index];
                return Entity {
                    .index = index,
                    .gen = newGen
                };
            }

            void remove(Entity e) {
                availableSlots.push_back(e.index);
                generations[e.index] = e.gen + 1;
            }

            bool isAlive(Entity e) {
                return generations[e.index] == e.gen;
            }
    };


}
#endif
