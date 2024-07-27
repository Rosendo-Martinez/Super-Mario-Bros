#include "EntityManager.h"

EntityManager::EntityManager()
{
}
void EntityManager::update() // removes dead entities & adds entities in wait list, should be called at begging of next frame (delayed affect)
{
}

std::shared_ptr<Entity> EntityManager::addEntity(const std::string& tag)
{
}

EntityVec& EntityManager::getEntities()
{
}

EntityVec& EntityManager::getEntities(const std::string& tag)
{
}
