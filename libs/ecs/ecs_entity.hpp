#pragma once
#include <cstddef>
#include <vector>
#include <cstdint>
#include "ecs_type.hpp"


namespace ecs {

    struct EntityRecord {
        uint32_t row{};
        ArchetypeID archetype = UINT64_MAX;

        void print() {
            std::cout << "Entity Row: " << row << ", Archetype ID: " << archetype << std::endl;
        }
    };

    class EntityManager {
    public:
        EntityManager() = default;

        inline Entity createEntity() {
            if (!availableEntities.empty()) {
                uint32_t index = availableEntities.back();
                availableEntities.pop_back();

                Entity entity{ index, generations[index] };
                entityRecords[index] = EntityRecord{};
                return entity;
            }

            uint32_t index = static_cast<uint32_t>(generations.size());
            generations.push_back(0);
            entityRecords.push_back(EntityRecord{ 0, UINT64_MAX });

            return Entity{ index, 0 };
        }

        inline void processEntityUpdate(EntityUpdate update) {
            getRecord(update.entity_index)->row = update.newRow;
        }


        inline bool isAlive(const Entity& e) const {
            return generations[e.index] == e.generation;
        }

        inline void destroyEntity(uint32_t index) {
            if (index >= generations.size()) return;

            generations[index]++;
            availableEntities.push_back(index);
            entityRecords[index] = EntityRecord{ 0, UINT64_MAX };
        }

        inline EntityRecord *getRecord(uint32_t index) {
            return &entityRecords[index];
        }

    private:
        std::vector<EntityRecord> entityRecords;
        std::vector<uint16_t> generations;
        std::vector<uint32_t> availableEntities;
    };

}
