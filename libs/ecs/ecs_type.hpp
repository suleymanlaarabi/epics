#pragma once
#include "span.hpp"
#include <cstdint>
#include <cstdio>
#include <vector>
#include <iostream>


namespace ecs {
    using u64 = uint64_t;
    using u32 = uint32_t;
    using ArchetypeID = u64;

    template<typename ...Components>
    using EcsIter = ZipSpan<Components...>;

    class World;

    struct ComponentRecord {
        size_t size;
    };

    struct Iter {
        u32 count;
        World *world;
    };

    struct QueryID {
        u64 id;

        QueryID() = default;
        QueryID(u64 value) : id(value) {}
    };

    struct EntityUpdate {
        u32 entity_index;
        u32 newRow;
    };

    #define ECS_PAIR_MASK (0x1ULL)

    struct Entity {
        uint32_t index;
        union {
            uint16_t generation;
            struct {
                uint32_t relationTarget;
            } relationTarget;
        };

        Entity() = default;
        Entity(uint32_t index, uint16_t generation)
            : index(index), generation(generation) {}

        uint64_t id() const noexcept {
            return (static_cast<uint64_t>(generation) << 32) | index;
        }

        bool isPair() const noexcept {
            return (id() & ECS_PAIR_MASK) != 0;
        }

        Entity makePair(Entity target) const {
            return Entity(index, target.index | ECS_PAIR_MASK);
        }

        bool operator==(const Entity& other) const noexcept {
            return this->id() == other.id();
        }

        auto operator<=>(const Entity& other) const noexcept {
            return this->id() <=> other.id();
        }

        friend std::ostream& operator<<(std::ostream& os, const Entity& entity) {
            os << "Entity(id: " << entity.id()
               << ", index: " << entity.index
               << ", generation: " << entity.generation << ")";
            return os;
        }
    };


    struct Type {
        std::vector<Entity> data;

        Type() = default;

        Type(Entity entity) {
            data.push_back(entity);
        }

        explicit Type(std::vector<Entity> d) : data(std::move(d)) {}

        void addComponent(Entity entity) {
            data.push_back(entity);
            this->sort();
        }

        void removeComponent(Entity entity) {
            data.erase(std::remove(data.begin(), data.end(), entity), data.end());
            this->sort();
        }

        void merge(Type type) {
            for (auto& term : type.data) {
                addComponent(term);
            }
        }

        bool includes(Entity entity) {
            return std::find(data.begin(), data.end(), entity) != data.end();
        }

        void sort() {
            std::sort(data.begin(), data.end());
        }

        bool operator==(const Type& other) const noexcept {
            return data == other.data;
        }

        bool matcheWith(const Type& other) const noexcept {
            if (data.empty()) return true;
            if (other.data.size() < data.size()) return false;

            auto it1 = data.begin();
            auto it2 = other.data.begin();

            while (it1 != data.end() && it2 != other.data.end()) {
                if (*it1 == *it2) {
                    ++it1;
                    ++it2;
                } else if (*it1 > *it2) {
                    ++it2;
                } else {
                    return false;
                }
            }
            return it1 == data.end();
        }

        void print() const {
            std::cout << "Type: ";
            for (const auto& entity : data) {
                std::cout << entity << " ";
            }
            std::cout << std::endl;
        }
    };

}

namespace std {
    template<>
    struct hash<ecs::Type> {
        std::size_t operator()(const ecs::Type& t) const noexcept {
            std::size_t h = 0;
            std::hash<ecs::u64> hasher;
            for (const auto& e : t.data) {
                h ^= hasher(e.id()) + 0x9e3779b9 + (h << 6) + (h >> 2);
            }
            return h;
        }
    };
}
