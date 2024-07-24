#include "Entity.h"

void Entity::destroy()
{
    m_active = false;
}

size_t Entity::id() const
{
    return m_id;
}

bool Entity::isActive() const
{
    return m_active;
}

const std::string & Entity::tag() const
{
    return m_tag;
}
