#ifndef ECS_FWD_HPP
    #define ECS_FWD_HPP
    #include <cstdint>
    #include <vector>
    #include "component.hpp"

namespace ecs {
    using Entity = uint64_t;

    class EntityType {
        std::vector<ComponentID> components;

        void normalize() {
            std::sort(components.begin(), components.end());
            components.erase(std::unique(components.begin(), components.end()), components.end());
        }

    public:
        template<typename... Components>
        void addComponents() {
            (components.push_back(getComponentID<Components>()), ...);
            normalize();
        }

        template<typename Component>
        void addComponent() {
            components.push_back(getComponentID<Component>());
            normalize();
        }

        void addComponent(ComponentID id) {
            components.push_back(id);
            normalize();
        }

        const std::vector<ComponentID>& getComponents() const {
            return components;
        }
    };

    struct EntityTypeHash {
        std::size_t operator()(const EntityType& v) const;
    };

    struct EntityTypeEq {
        bool operator()(const EntityType& a, const EntityType& b) const;
    };
}

#endif
