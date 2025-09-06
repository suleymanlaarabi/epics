#pragma once
#include <ecs_type.hpp>
#include <vector>

namespace ecs {
    class Query {

        public:
            Type terms;
            std::vector<ArchetypeID> matches;

            Query() = default;
            Query(Type terms) : terms(std::move(terms)) {}

            void addArchetype(ArchetypeID id) {
                matches.push_back(id);
            }
    };
}
