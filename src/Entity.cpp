#include "Entity.h"

Entity::Entity(const size_t & id, const std::string & tag)
    : m_active(id)
    , m_tag(tag)
{
}

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
