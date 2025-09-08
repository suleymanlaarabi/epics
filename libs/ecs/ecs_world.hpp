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
    using ObserverFunc = void(*)(Entity, ArchetypeID, World *);

    enum class ObserverType {
        OnAdded,
        OnRemoved,
        OnChanged
    };

    class Observer {
        QueryID query;
        ObserverFunc func;
    };

    class EcsFunc {

        public:
            FuncType func;
            EcsFunc(FuncType func) : func(func) {}

            void printFuncPtr() const {
                printf("Function pointer: %p\n", func);
            }
    };



    struct RegisteredPlugin {};
    struct ChildOf {};
    struct DependsOn {};

    struct OnPreUpdate {};
    struct OnUpdate {};
    struct OnPostUpdate {};

    class Plugin {
        public:
            virtual ~Plugin() = default;
            virtual void build(ecs::World& world) = 0;
    };

    template<typename... IterComponents>
    class QueryBuilder;

    class World {
        EntityManager entity_manager;
        SparseSet<Entity> components;
        SparseSet<ComponentRecord> component_records;
        std::vector<Archetype> archetypes;
        std::unordered_map<Type, ArchetypeID> archetype_map;
        std::vector<Query> queries;
        std::unordered_map<Query, QueryID, QueryHash> query_map;

        QueryID onPreUpdateSystemsQuery;
        QueryID onUpdateSystemsQuery;
        QueryID onPostUpdateSystemsQuery;

        private:
            ArchetypeID getOrCreateArchetypeID(Type type);
            void migrateEntity(ArchetypeID oldArchetypeID, ArchetypeID newArchetypeID, size_t oldRow);
            Query &getSystemsQuery();
            std::vector<ArchetypeID> &getQueryMatchedArchetypes(QueryID queryID);
            void runSystemsPhase(QueryID phase);

            template<typename ...Components>
            Type createType() {
                return Type{ std::vector<Entity>{ component<Components>()... } };
            }

            inline QueryID registerQuery(Query &newQuery) {
                if (query_map.contains(newQuery)) {
                    return query_map.at(newQuery);
                }

                queries.push_back(newQuery);
                Query &query = queries.back();

                for (std::size_t i = 0; i < archetypes.size(); ++i) {
                    auto& archetype = archetypes[i];
                    if (query.matchWithType(archetype.type)) {
                        query.addArchetype(i);
                    }
                }
                query_map[query] = QueryID {queries.size() - 1};

                return QueryID {queries.size() - 1};
            }

            inline QueryID registerQuery(Type type) {
                Query query = Query(type);
                return registerQuery(query);
            }


        public:
            const Entity ChildOfRelation = component<ChildOf>();
            Entity entity();
            Entity entity(size_t size);
            void pair(Entity entity, Entity relation, Entity target);
            void childOf(Entity child, Entity parent);
            void dependsOn(Entity entity, Entity target);
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
            Entity registerSystem(FuncType func, QueryID query);
            void progress();
            void run();

            World();

            void debug() {
                std::cout << "Query Length: " << queries.size() << std::endl;
            }

            template <typename Plugin>
            void plugin(Plugin &&plugin) {
                Entity pluginEntity = component<Plugin>();

                if (has<RegisteredPlugin>(pluginEntity)) {
                    return;
                }

                add<RegisteredPlugin>(pluginEntity);

                plugin.build(*this);
            }

            template<typename Component>
            void singleton() {
                add<Component>(component<Component>());
            }

            template<typename Component>
            Component *getSingleton() {
                Entity entity = component<Component>();
                EntityRecord *record = entity_manager.getRecord(entity.index);

                return static_cast<Component *>(archetypes[record->archetype].getRawComponent(record->row, entity));
            }

            template<typename Component>
            void setSingleton(Component *value) {
                set(component<Component>(), value);
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
            Entity registerSystem(FuncType func) {
                QueryID queryID = query<Components...>();
                return registerSystem(func, queryID);
            }


            template<typename... Components, typename Func>
            requires std::invocable<Func, ZipSpan<Components...>>
            Entity system(Func&& func, Query query) {
                static Func userFunc = std::forward<Func>(func);

                FuncType invoker = [](ArchetypeID archetypeID, World* world) {
                    std::invoke(userFunc, world->iter<Components...>(archetypeID));
                };

                query.mergeTerms(createType<Components...>());

                return registerSystem(invoker, registerQuery(query));
            }


            template<typename... Components>
            QueryBuilder<Components...> system() {
                return QueryBuilder<Components...>(this);
            }

            template<typename... Components, typename Func>
            requires std::invocable<Func, Iter, Components*...>
            Entity systemIter(Func&& func, Query query) {
                static Func userFunc = std::forward<Func>(func);

                FuncType invoker = [](ArchetypeID archetypeID, World* world) {
                    Archetype *archetype = &world->archetypes[archetypeID];
                    std::invoke(userFunc,
                        Iter(archetype->entityCount(), world, archetypeID),
                        reinterpret_cast<Components* __restrict>(
                            archetype->getRawComponent(0, world->component<Components>())
                        )...
                    );
                };

                query.mergeTerms(createType<Components...>());

                return registerSystem(invoker, registerQuery(query));
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
            inline void remove(Entity entity) {
                Entity componentEntity = component<Component>();
                remove(entity, componentEntity);
            }

            template<typename Component>
            inline void add(Entity entity) {
                Entity componentEntity = component<Component>();
                add(entity, componentEntity);
            }

            template<typename Component>
            void childOf(Entity entity) {
                Entity componentEntity = component<Component>();
                childOf(entity, componentEntity);
            }

            template<typename Component>
            void dependsOn(Entity entity) {
                Entity componentEntity = component<Component>();
                dependsOn(entity, componentEntity);
            }

            template<typename Component>
            void set(Entity entity, Component&& value) {
                Entity componentEntity = component<Component>();

                auto *storage = new Component(std::forward<Component>(value));

                set(entity, componentEntity, static_cast<void*>(storage));
            }

            template<typename Component>
            Component &get(Entity entity) {
                EntityRecord *record = entity_manager.getRecord(entity.index);
                Entity componentEntity = component<Component>();

                return *static_cast<Component *>(get(entity, componentEntity));
            }

    };
}
