#pragma once
    #include "ecs_archetype.hpp"
    #include "ecs_entity.hpp"
    #include "ecs_query.hpp"
    #include "ecs_type.hpp"
    #include "rayflect.hpp"
    #include "span.hpp"
    #include "sparseset.hpp"
    #include <cstddef>
    #include <cstdio>
    #include <tuple>
    #include <unordered_map>
    #include <vector>
    #define ECS_PLUGIN(Derived) struct Derived : public ecs::PluginBase<Derived>


namespace ecs {

    class World;

    using FuncType = void(*)(ArchetypeID, World *);

    class EcsFunc {

        public:
            FuncType func;
            EcsFunc(FuncType func) : func(func) {}

            void printFuncPtr() const {
                printf("Function pointer: %p\n", func);
            }
    };

    class RegisteredPlugin {};

    class Plugin {
        public:
            virtual ~Plugin() = default;
            virtual void build(ecs::World& world) = 0;
    };

    struct ChildOf {};

    class World {
        EntityManager entity_manager;
        SparseSet<Entity> components;
        SparseSet<ComponentRecord> component_records;
        std::vector<Archetype> archetypes;
        std::unordered_map<Type, ArchetypeID> archetype_map;
        std::vector<Query> queries;
        std::unordered_map<Type, QueryID> query_map;
        QueryID systems_query;

        private:
            ArchetypeID getOrCreateArchetypeID(Type type);
            void migrateEntity(ArchetypeID oldArchetypeID, ArchetypeID newArchetypeID, size_t oldRow);
            Query &getSystemsQuery();
            std::vector<ArchetypeID> &getQueryMatchedArchetypes(QueryID queryID);


            template<typename ...Components>
            Type createType() {
                Type type;
                (type.addComponent(component<Components>()), ...);
                return type;
            }

            QueryID registerQuery(Type type) {
                if (query_map.contains(type)) {
                    return query_map[type];
                }

                queries.push_back(Query(type));
                Query &query = queries.back();

                for (std::size_t i = 0; i < archetypes.size(); ++i) {
                    auto& archetype = archetypes[i];
                    if (query.terms.matcheWith(archetype.type)) {
                        query.addArchetype(i);
                    }
                }

                query_map[type] = QueryID {queries.size() - 1};

                return QueryID {queries.size() - 1};
            }


        public:
            const Entity ChildOfRelation = component<ChildOf>();
            Entity entity();
            Entity entity(size_t size);
            void pair(Entity entity, Entity relation, Entity target);
            void childOf(Entity child, Entity parent);
            Entity relation(Entity relation, Entity target);
            void kill(Entity entity);
            bool has(Entity entity, Entity component);
            bool isAlive(Entity entity);
            void add(Entity entity, Entity component);
            void remove(Entity entity, Entity component);
            void set(Entity entity, Entity component, void *value);
            void *get(Entity entity, Entity component);
            std::vector<ArchetypeID> &getSystems();
            Archetype *getArchetype(ArchetypeID archetypeID);
            Archetype *getArchetype(Entity entity);
            void registerSystem(FuncType func, QueryID query);
            void progress();

            World();

            template <typename Plugin>
            void plugin(Plugin &&plugin) {
                Entity pluginEntity = component<Plugin>();

                if (has<RegisteredPlugin>(pluginEntity)) {
                    return;
                }

                add<RegisteredPlugin>(pluginEntity);

                plugin.build(*this);
            }

            template<typename ...Components>
            QueryID query() {
                Type type = createType<Components...>();

                return registerQuery(type);
            }

            template<typename ...Components>
            ZipSpan<Components...> iter(ArchetypeID id) {
                Archetype &archetype = archetypes[id];
                ZipSpan<Components...> zip = {
                    std::tuple<Components*...>{
                        reinterpret_cast<Components*>(archetype.getRawComponent(0, component<Components>()))...
                    },
                    archetype.entityCount()
                };

                return zip;
            }

            template<typename ...Components>
            void registerSystem(FuncType func) {
                QueryID queryID = query<Components...>();
                registerSystem(func, queryID);
            }

            template<typename... Components, typename Func>
            requires std::invocable<Func, ZipSpan<Components...>>
            void system(Func&& func) {
                static Func userFunc = std::forward<Func>(func);

                FuncType invoker = [](ArchetypeID archetypeID, World* world) {
                    std::invoke(userFunc, world->iter<Components...>(archetypeID));
                };

                registerSystem<Components...>(invoker);
            }

            template<typename... Components, typename Func>
            requires std::invocable<Func, ZipSpan<Components...>>
            void system(Func&& func, Type otherTerms) {
                static Func userFunc = std::forward<Func>(func);

                FuncType invoker = [](ArchetypeID archetypeID, World* world) {
                    std::invoke(userFunc, world->iter<Components...>(archetypeID));
                };

                Type terms = createType<Components...>();

                terms.merge(otherTerms);

                registerSystem(invoker, registerQuery(terms));
            }

            template<typename... Components, typename Func>
            requires std::invocable<Func, u32, Components*...>
            void systemIter(Func&& func) {
                static Func userFunc = std::forward<Func>(func);

                FuncType invoker = [](ArchetypeID archetypeID, World* world) {
                    Archetype *archetype = &world->archetypes[archetypeID];
                    std::invoke(userFunc,
                        archetype->entityCount(),
                        reinterpret_cast<Components*>(
                            archetype->getRawComponent(0, world->component<Components>())
                        )...
                    );
                };

                registerSystem<Components...>(invoker);
            }

            template<typename... Components, typename Func>
            requires std::invocable<Func, u32, Components*...>
            void systemIter(Func&& func, Type otherTerms) {
                static Func userFunc = std::forward<Func>(func);

                FuncType invoker = [](ArchetypeID archetypeID, World* world) {
                    Archetype *archetype = &world->archetypes[archetypeID];
                    std::invoke(userFunc,
                        archetype->entityCount(),
                        reinterpret_cast<Components* __restrict>(
                            archetype->getRawComponent(0, world->component<Components>())
                        )...
                    );
                };

                Type terms = createType<Components...>();

                terms.merge(otherTerms);

                registerSystem(invoker, registerQuery(terms));
            }

            template<typename Component>
            Entity component() {
                rayflect::ComponentID id = rayflect::getComponentID<Component>();

                if (components.has(id)) {
                    return components.get(id);
                }

                Entity entity = entity_manager.createEntity();
                component_records.set(entity.id(), ComponentRecord {sizeof(Component)});

                components.set(id, entity);
                return entity;
            }

            template<typename Component>
            bool has(Entity entity) {
                Entity componentEntity = component<Component>();
                return has(entity, componentEntity);
            }

            template<typename Component>
            void remove(Entity entity) {
                Entity componentEntity = component<Component>();
                remove(entity, componentEntity);
            }

            template<typename Component>
            void add(Entity entity) {
                Entity componentEntity = component<Component>();
                add(entity, componentEntity);
            }

            template<typename Component>
            void childOf(Entity entity) {
                Entity componentEntity = component<Component>();
                childOf(entity, componentEntity);
            }

            template<typename Component>
            void set(Entity entity, Component&& value) {
                Entity componentEntity = component<Component>();
                set(entity, componentEntity, static_cast<void*>(&value));
            }

            template<typename Component>
            Component &get(Entity entity) {
                EntityRecord *record = entity_manager.getRecord(entity.index);
                Entity componentEntity = component<Component>();

                return *static_cast<Component *>(get(entity, componentEntity));
            }

    };
}
