#pragma once
#include <ecs_type.hpp>
#include <unordered_set>
#include <vector>

namespace ecs {

    enum class EcsAccess {
        In,
        Out,
        InOut,
        None
    };

    class Query {
        public:
            Type terms;
            Type notTerms;
            std::unordered_set<Entity> read_access;
            std::unordered_set<Entity> none_access;
            std::vector<ArchetypeID> matches;
            Query() = default;
            Query(Type terms) : terms(std::move(terms)) {}
            Query(Type terms, Type notTerms) : terms(std::move(terms)), notTerms(std::move(notTerms)) {}

            void print() {
                std::cout << "Query: ";
                terms.print();
                std::cout << " NOT ";
                notTerms.print();
                std::cout << std::endl;
            }

            void addArchetype(ArchetypeID id) {
                matches.push_back(id);
            }

            void addTerm(Entity term) {
                terms.addComponent(term);
            }

            void readTerm(Entity term) {
                read_access.insert(term);
            }

            void noneTerm(Entity term) {
                none_access.insert(term);
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

    inline bool operator==(const Query& a, const Query& b) {
        return a.terms == b.terms && a.notTerms == b.notTerms;
    }

    struct QueryHash {
        std::size_t operator()(const Query& q) const noexcept {
            std::size_t h1 = std::hash<Type>{}(q.terms);
            std::size_t h2 = std::hash<Type>{}(q.notTerms);
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };

}
