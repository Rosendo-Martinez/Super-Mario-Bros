#include "Animation.h"
#include <cmath>

Animation::Animation()
{
}

Animation::Animation(const std::string & name, const sf::Texture & t)
    : Animation(name, t, 1, 0)
{
}

Animation::Animation(const std::string & name, const sf::Texture & t, size_t frameCount, size_t speed)
    : m_name(name)
    , m_sprite(t)
    , m_frameCount(frameCount)
    , m_currentFrame(0)
    , m_speed(speed)
{
    m_size = Vec2((float)t.getSize().x / frameCount, (float)t.getSize().y);
    m_sprite.setOrigin(m_size.x / 2.0f, m_size.y / 2.0f);
    m_sprite.setTextureRect(sf::IntRect(0, 0, m_size.x, m_size.y));
}

// updates the animation to show the next frame, depending on its speed
// animation loop when it reaches the end
void Animation::update()
{
    m_currentFrame++;

    // TODO: 1) calc correct frame of animation
    //       2) set the texture rect
}

bool Animation::hasEnded() const
{
    // TODO: detect when animation has ended (last frame was played) and return true
    return false;
}

const std::string & Animation::getName() const
{
    return m_name;
}

const Vec2 & Animation::getSize() const
{
    return m_size;
}

const sf::Sprite & Animation::getSprite() const
{
    return m_sprite;
}
