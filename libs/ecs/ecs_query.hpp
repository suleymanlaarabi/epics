#pragma once
#include <ecs_type.hpp>
#include <vector>

namespace ecs {
    class Query {

        public:
            Type terms;
            Type notTerms;
            std::vector<ArchetypeID> matches;

            Query() = default;
            Query(Type terms) : terms(std::move(terms)) {}
            Query(Type terms, Type notTerms) : terms(std::move(terms)), notTerms(std::move(notTerms)) {}

            void addArchetype(ArchetypeID id) {
                matches.push_back(id);
            }

            void addTerm(Entity term) {
                terms.addComponent(term);
            }

            void addNotTerm(Entity term) {
                notTerms.addComponent(term);
            }

            void mergeTerms(Type terms) {
                this->terms.merge(terms);
            }

            bool matchWithType(Type &other) {
                if (!terms.matcheWith(other)) {
                    return false;
                }

                for (auto &notTerm : notTerms.data) {
                    if (other.includes(notTerm)) {
                        return false;
                    }
                }

                return true;
            }
    };
}
