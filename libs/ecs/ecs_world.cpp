#include "ecs_query.hpp"
#include "ecs_type.hpp"
#include <cstdio>
#include <ecs_world.hpp>

using namespace ecs;

World::World() {
    auto q = query<QueryID, EcsFunc>();
    systems_query = q;
}

ArchetypeID World::getOrCreateArchetypeID(Type type) {
    if (archetype_map.contains(type)) {
        return archetype_map[type];
    }
    ArchetypeID id = archetypes.size();
    archetype_map[type] = id;
    archetypes.push_back(Archetype(type, &component_records));

    for (auto &query: queries) {
        if (query.terms.matcheWith(type)) {
            query.addArchetype(id);
        }
    }

    return id;
}

void World::migrateEntity(ArchetypeID oldArchetypeID, ArchetypeID newArchetypeID, size_t oldRow) {
    Archetype *oldArchetype = &archetypes[oldArchetypeID];
    Archetype *newArchetype = &archetypes[newArchetypeID];
    size_t newRow = newArchetype->entities_indices.size() - 1;

    for (auto &component : oldArchetype->type.data) {
        if (!newArchetype->has_component(component)) {
            continue;
        }
        newArchetype->writeRawComponent(newRow, component, oldArchetype->getRawComponent(oldRow, component));
    }
}

Entity World::entity() {
    return entity_manager.createEntity();
}

Entity World::entity(size_t size) {
    Entity entity = entity_manager.createEntity();
    component_records.set(entity.id(), ComponentRecord {size});
    return entity;
}

void World::kill(Entity entity) {
    return entity_manager.destroyEntity(entity.index);
}

bool World::has(Entity entity, Entity component) {
    ArchetypeID archetypeID = entity_manager.getRecord(entity.index)->archetype;

    if (archetypeID == UINT64_MAX) {
        return false;
    }

    return archetypes[archetypeID].has_component(component);
}

bool World::isAlive(Entity entity) {
    return entity_manager.isAlive(entity);
}

void World::add(Entity entity, Entity component) {
    EntityRecord *record = entity_manager.getRecord(entity.index);

    if (record->archetype == UINT64_MAX) {
        record->archetype = getOrCreateArchetypeID(Type(component));
        record->row = archetypes[record->archetype].addEntity(entity);
        return;
    }

    if (archetypes[record->archetype].has_component(component)) {
        return;
    }

    ArchetypeID newArchetypeID = archetypes[record->archetype].getAddEdge(component);

    if (newArchetypeID == UINT64_MAX) {
        Type newType = archetypes[record->archetype].type;
        newType.addComponent(component);
        newArchetypeID = getOrCreateArchetypeID(newType);

        archetypes[record->archetype].setAddEdge(component, newArchetypeID);
        archetypes[newArchetypeID].setRemoveEdge(component, record->archetype);
    }

    size_t newRow = archetypes[newArchetypeID].addEntity(entity);

    migrateEntity(record->archetype, newArchetypeID, record->row);

    entity_manager.processEntityUpdate(archetypes[record->archetype].removeEntity(record->row));

    record->archetype = newArchetypeID;
    record->row = newRow;
}

void World::remove(Entity entity, Entity component) {
    EntityRecord *record = entity_manager.getRecord(entity.index);

    if (record->archetype == UINT64_MAX || !archetypes[record->archetype].has_component(component)) {
        return;
    }

    ArchetypeID newArchetypeID = archetypes[record->archetype].getRemoveEdge(component);

    if (newArchetypeID == UINT64_MAX) {
        Type newType = archetypes[record->archetype].type;

        newType.removeComponent(component);
        newArchetypeID = getOrCreateArchetypeID(newType);

        archetypes[record->archetype].setRemoveEdge(component, newArchetypeID);
        archetypes[newArchetypeID].setAddEdge(component, record->archetype);
    }

    size_t newRow = archetypes[newArchetypeID].addEntity(entity);

    migrateEntity(record->archetype, newArchetypeID, newRow);

    entity_manager.processEntityUpdate(archetypes[record->archetype].removeEntity(record->row));

    record->archetype = newArchetypeID;
    record->row = newRow;
}

void World::set(Entity entity, Entity component, void *value) {
    if (!has(entity, component)) {
        add(entity, component);
    }

    EntityRecord *record = entity_manager.getRecord(entity.index);

    archetypes[record->archetype].writeRawComponent(record->row, component, value);
}

void *World::get(Entity entity, Entity component) {
    EntityRecord *record = entity_manager.getRecord(entity.index);

    return archetypes[record->archetype].getRawComponent(record->row, component);
}

std::vector<ArchetypeID> &World::getQueryMatchedArchetypes(QueryID queryID) {
    return queries[queryID.id].matches;
}

std::vector<ArchetypeID> &World::getSystems() {
    return queries[systems_query.id].matches;
}

Query &World::getSystemsQuery() {
    return queries[systems_query.id];
}

Archetype *World::getArchetype(ArchetypeID archetypeID) {
    return &archetypes[archetypeID];
}

Archetype *World::getArchetype(Entity entity) {
    EntityRecord *record = entity_manager.getRecord(entity.index);

    return &archetypes[record->archetype];
}

void World::registerSystem(FuncType func, QueryID query) {
    Entity system = entity();


    set<EcsFunc>(system, EcsFunc(func));
    set<QueryID>(system, QueryID(query.id));
}

void World::progess() {
    std::vector<ecs::ArchetypeID> &archetypes = getSystems();
    for (auto archetypeID : archetypes) {
        auto zip = iter<ecs::EcsFunc, ecs::QueryID>(archetypeID);

        for (auto [func, query] : zip) {
            for (auto archetypeID : getQueryMatchedArchetypes(query)) {
                func.func(archetypeID, this);
            }
        }
    }
}

void World::pair(Entity entity, Entity relation, Entity target) {
    Entity relationEntity = relation.makePair(target);

    add(entity, relationEntity);
}

void World::childOf(Entity child, Entity parent) {
    pair(child, ChildOfRelation, parent);
}

Entity World::relation(Entity relation, Entity target) {
    return relation.makePair(target);
}
