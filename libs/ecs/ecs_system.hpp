#pragma once

#include "ecs_query.hpp"
#include "ecs_type.hpp"
#include "ecs_world.hpp"

namespace ecs {

    template<typename... IterComponents>
    class QueryBuilder {
        private:
            Query query;
            World *world;

        public:
            QueryBuilder(World *world) : world(world) {}
            QueryBuilder(World* world, Query q) : world(world), query(std::move(q)) {}



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
                return QueryBuilder<IterComponents..., const Component>(world, query);
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
            void each(Func&& func) {
                world->system<IterComponents...>(func, query);
            }

            template<typename Func>
            requires std::invocable<Func, Iter, IterComponents*...>
            void iter(Func&& func) {
                world->systemIter<IterComponents...>(func, query);
            }

    };
}
