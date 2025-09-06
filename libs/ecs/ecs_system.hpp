#pragma once


#include "ecs_query.hpp"
#include "ecs_type.hpp"
#include "ecs_world.hpp"

namespace ecs {
    class QueryBuilder {
        private:
            Type terms;
            World *world;

        public:
            QueryBuilder(World *world) : world(world) {}

            QueryBuilder& childOf(Entity entity) {
                terms.addComponent(world->relation(world->component<ChildOf>(), entity));
                return *this;
            }

            void merge(Type other) {
                for (auto& component : other.data) {
                    terms.addComponent(component);
                }
            }

            Query asQuery() {
                return Query(terms);
            }

            template<typename... Components, typename Func>
            requires std::invocable<Func, ZipSpan<Components...>>
            void each(Func&& func) {
                world->system<Components...>(std::forward<Func>(func), terms);
            }

    };
}
