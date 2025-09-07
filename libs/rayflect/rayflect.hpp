#ifndef RAYFLECT_H
    #define RAYFLECT_H
    #include <cstddef>
    #include <cstdint>
    #include <functional>
    #include <unordered_map>
    #include <mutex>

namespace rayflect {

    using ComponentID = uint64_t;

    namespace detail {
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

        inline std::unordered_map<ComponentID, std::size_t> getRegisteredComponents() {
            std::lock_guard<std::mutex> lock(componentSizeTableMutex());
            return componentSizeTable();
        }
    }

    inline void forEachRegisteredComponent(
        const std::function<void(ComponentID, std::size_t)>& fn)
    {
        std::lock_guard<std::mutex> lock(detail::componentSizeTableMutex());
        for (const auto& [id, size] : detail::componentSizeTable()) {
            fn(id, size);
        }
    }

    template <typename T>
    ComponentID _getComponentID() {
        static ComponentID typeID = [] {
            ComponentID id = detail::createComponentID();
            std::lock_guard<std::mutex> lock(detail::componentSizeTableMutex());
            detail::componentSizeTable()[id] = sizeof(T);
            return id;
        }();
        return typeID;
    }

    template <typename T>
    ComponentID getComponentID() {
        using DecayedT = std::remove_cv_t<std::remove_reference_t<T>>;
        return _getComponentID<DecayedT>();
    }

    inline ComponentID getComponentID() {
        static ComponentID typeID = detail::createComponentID();
        std::lock_guard<std::mutex> lock(detail::componentSizeTableMutex());
        detail::componentSizeTable()[typeID] = 0;
        return typeID;
    }

    inline std::size_t getComponentSizeByID(ComponentID id) {
        std::lock_guard<std::mutex> lock(detail::componentSizeTableMutex());
        auto it = detail::componentSizeTable().find(id);
        if (it != detail::componentSizeTable().end())
            return it->second;
        return 0;
    }

}

#endif
