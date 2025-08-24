#ifndef ECS_TYPE_H
    #define ECS_TYPE_H
    #include <rayflect.hpp>
    #include <vector>

namespace ecs {
    using ComponentID = rayflect::ComponentID;
    using EntityType = std::vector<ComponentID>;
}

#endif
