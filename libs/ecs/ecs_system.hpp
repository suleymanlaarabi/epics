#pragma once

#include "ecs_query.hpp"
#include "ecs_type.hpp"
#include "ecs_world.hpp"

namespace ecs {
    class QueryBuilder {
        private:
            Query query;
            World *world;

        public:
            QueryBuilder(World *world) : world(world) {}

            QueryBuilder& childOf(Entity entity) {
                query.addTerm(world->relation(world->component<ChildOf>(), entity));
                return *this;
            }

            template<typename Component>
            QueryBuilder& childOf() {
                this->childOf(world->component<Component>());
                return *this;
            }

            QueryBuilder& with(Entity entity) {
                query.addTerm(entity);
                return *this;
            }

            template<typename ...Components>
            QueryBuilder& with() {
                (query.addTerm(world->component<Components>()), ...);
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

            template<typename... Components, typename Func>
            requires std::invocable<Func, ZipSpan<Components...>>
            void each(Func&& func) {
                world->system<Components...>(func, query);
            }

            template<typename... Components, typename Func>
            requires std::invocable<Func, u32, Components*...>
            void iter(Func&& func) {
                world->systemIter<Components...>(func, query);
            }

    };
}
