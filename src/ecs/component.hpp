#ifndef ECS_COMPONENT_HPP
    #define ECS_COMPONENT_HPP
    #include <cstddef>
    #include <cstdint>
    #include <unordered_map>
    #include <mutex>

namespace ecs {

    using ComponentID = uint64_t;

    inline ComponentID createComponentID() {
        static ComponentID lastID = 0;
        return lastID++;
    }

    inline std::unordered_map<ComponentID, std::size_t>& componentSizeTable() {
        static std::unordered_map<ComponentID, std::size_t> table;
        return table;
    }

    inline std::mutex& componentSizeTableMutex() {
        static std::mutex m;
        return m;
    }

    template <typename T>
    ComponentID getComponentID() {
        static ComponentID typeID = [] {
            ComponentID id = createComponentID();
            std::lock_guard<std::mutex> lock(componentSizeTableMutex());
            componentSizeTable()[id] = sizeof(T);
            return id;
        }();
        return typeID;
    }

    inline std::size_t getComponentSizeByID(ComponentID id) {
        std::lock_guard<std::mutex> lock(componentSizeTableMutex());
        auto it = componentSizeTable().find(id);
        if (it != componentSizeTable().end())
            return it->second;
        return 0;
    }

}

#endif
