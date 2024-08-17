#include "EntityManager.h"

EntityManager::EntityManager()
{
}
void EntityManager::update() // removes dead entities & adds entities in wait list, should be called at begging of next frame (delayed affect)
{
    // add entities on wait list
    for (auto e : m_toAdd)
    {
        m_entities.push_back(e);
        m_entityMap[e->tag()].push_back(e);
    }
    m_toAdd.clear();

    // TODO: remove dead entities
}

std::shared_ptr<Entity> EntityManager::addEntity(const std::string& tag)
{
    m_totalEntities++;

    auto e = std::shared_ptr<Entity>(new Entity(m_totalEntities, tag));
    m_toAdd.push_back(e);
    return e;
}

EntityVec& EntityManager::getEntities()
{
    return m_entities;
}

EntityVec& EntityManager::getEntities(const std::string& tag)
{
}

size_t EntityManager::getTotalEntitiesCreated()
{
    return m_totalEntities;
}