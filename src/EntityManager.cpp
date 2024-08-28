#include "EntityManager.h"

EntityManager::EntityManager()
{
}
void EntityManager::update()
{
    // Be careful of iterator invalidation!
    // Modifying a list while iterating through it will invalidate the iterator.
    // This is why special care is needed when removing and adding entities.

    // add entities on wait list
    for (auto e : m_toAdd)
    {
        m_entities.push_back(e);
        m_entityMap[e->tag()].push_back(e);
    }
    m_toAdd.clear();

    // Remove dead entities from entity list
    EntityVec::iterator it = std::remove_if(m_entities.begin(), m_entities.end(), [](const std::shared_ptr<Entity> e){ return !e->isActive(); });
    m_entities.erase(it, m_entities.end());

    // Remove dead entities from entity map
    for (auto& p : m_entityMap)
    {
        EntityVec::iterator it = std::remove_if(p.second.begin(), p.second.end(), [](const std::shared_ptr<Entity> e){ return !e->isActive(); });
        p.second.erase(it, p.second.end());
    }
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
    return m_entityMap[tag];
}

size_t EntityManager::getTotalEntitiesCreated()
{
    return m_totalEntities;
}