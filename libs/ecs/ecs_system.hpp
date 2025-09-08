#pragma once

#include "ecs_query.hpp"
#include "ecs_type.hpp"
#include "ecs_world.hpp"
#include <vector>

namespace ecs {

    template<typename... IterComponents>
    class QueryBuilder {
        private:
            Query query;
            std::vector<Entity> systemComponents;
            World *world;

        public:
            QueryBuilder(World *world) : world(world) {}
            QueryBuilder(World* world, Query q, std::vector<Entity> systemComponents) : world(world), query(std::move(q)), systemComponents(std::move(systemComponents)) {}

            QueryBuilder& debug() {
                query.print();
                return *this;
            }

            QueryBuilder& childOf(Entity entity) {
                query.addTerm(world->relation(world->component<ChildOf>(), entity));
                return *this;
            }

            template<typename Component>
            QueryBuilder& childOf() {
                this->childOf(world->component<Component>());
                return *this;
            }

            template<typename Component>
            QueryBuilder& on() {
                systemComponents.push_back(world->component<Component>());
                return *this;
            }

            QueryBuilder& with(Entity entity) {
                query.addTerm(entity);
                query.noneTerm(entity);
                return *this;
            }

            QueryBuilder& read(Entity entity) {
                query.readTerm(entity);
                return *this;
            }

            template<typename Component>
            QueryBuilder<IterComponents..., const Component> read() {
                query.readTerm(world->component<Component>());
                return QueryBuilder<IterComponents..., const Component>(world, query, systemComponents);
            }

            template<typename ...Components>
            QueryBuilder& with() {
                (query.addTerm(world->component<Components>()), ...);
                (query.noneTerm(world->component<Components>()), ...);
                return *this;
            }

            QueryBuilder& without(Entity entity) {
                query.addNotTerm(entity);
                return *this;
            }

            template<typename ...Components>
            QueryBuilder& without() {
                (query.addNotTerm(world->component<Components>()), ...);
                return *this;
            }

            template<typename Func>
            requires std::invocable<Func, ZipSpan<IterComponents...>>
            Entity each(Func&& func) {
                Entity system = world->system<IterComponents...>(func, query);
                for (Entity entity : systemComponents) {
                    world->add(system, entity);
                }
                return system;
            }

            template<typename Func>
            requires std::invocable<Func, Iter, IterComponents*...>
            Entity iter(Func&& func) {
                Entity system = world->systemIter<IterComponents...>(func, query);
                for (Entity entity : systemComponents) {
                    world->add(system, entity);
                }
                return system;
            }

    };
}
