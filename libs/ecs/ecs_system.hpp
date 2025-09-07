#pragma once

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

            template<typename Component>
            QueryBuilder& childOf() {
                this->childOf(world->component<Component>());
                return *this;
            }

            QueryBuilder& with(Entity entity) {
                terms.addComponent(entity);
                return *this;
            }

            template<typename ...Components>
            QueryBuilder& with() {
                (terms.addComponent(world->component<Components>()), ...);
                return *this;
            }

            template<typename... Components, typename Func>
            requires std::invocable<Func, ZipSpan<Components...>>
            void each(Func&& func) {
                world->system<Components...>(func, terms);
            }

            template<typename... Components, typename Func>
            requires std::invocable<Func, u32, Components*...>
            void iter(Func&& func) {
                world->systemIter<Components...>(func, terms);
            }

    };
}
