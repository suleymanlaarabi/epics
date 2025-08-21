#include "world.hpp"

std::size_t ecs::EntityTypeHash::operator()(const EntityType& v) const {
    std::size_t h = 0;
    for (auto id : v.getComponents()) {
        h ^= std::hash<ComponentID>{}(id) + 0x9e3779b9 + (h << 6) + (h >> 2);
    }
    return h;
}

bool ecs::EntityTypeEq::operator()(const EntityType& a, const EntityType& b) const {
    return a.getComponents() == b.getComponents();
}
