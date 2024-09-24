#include "Animation.h"
#include <cmath>
#include <cassert>

Animation::Animation()
{
}

Animation::Animation(const std::string & name, const sf::Texture & t)
    : Animation(name, t, 1, 0)
{
}

Animation::Animation(const std::string & name, const sf::Texture & t, size_t duration) // Duration is same as speed for single-frame textures
    : Animation(name, t, 1, duration)
{
}

Animation::Animation(const std::string & name, const sf::Texture & t, size_t frameCount, size_t speed)
    : Animation(name, t, frameCount, speed, 1.f, 1.f)
{
}

Animation::Animation(const std::string & name, const sf::Texture & t, size_t frameCount, size_t speed, float scaleX, float scaleY)
    : m_name(name)
    , m_sprite(t)
    , m_frameCount(frameCount)
    , m_currentFrame(0)
    , m_speed(speed)
{
    assert(frameCount > 1 ? (speed > 0) : true); // Speed must be non-zero for multi-frame assets
    m_size = Vec2((float)t.getSize().x / frameCount, (float)t.getSize().y);
    m_sprite.setOrigin(m_size.x / 2.0f, m_size.y / 2.0f);
    m_sprite.setTextureRect(sf::IntRect(0, 0, m_size.x, m_size.y));
    m_sprite.setScale(sf::Vector2f(scaleX, scaleY));
}

// updates the animation to show the next frame, depending on its speed
// animation loop when it reaches the end
void Animation::update()
{
    m_currentFrame++;

    if (m_frameCount > 1)
    {
        // zero-indexed frame count
        int fullAnimationFramesPlayed = (int) floor(m_currentFrame / m_speed);
        // zero-indexed animation frames
        int currentAnimationFrame = fullAnimationFramesPlayed % m_frameCount;

        m_sprite.setTextureRect(sf::IntRect(currentAnimationFrame * m_size.x, 0, m_size.x, m_size.y));
    }
}

/*
    Returns true if all texture frames have been fully played.
    A texture frame has been fully played if it has been played for X
    frames, where X is equal to the speed/duration. Else, it returns 
    false.

    Note, a speed or duration of 0 always returns true.
*/
bool Animation::hasEnded() const
{
    if (m_speed == 0 || floor(m_currentFrame / m_speed) > m_frameCount)
    {
        return true;
    }
    
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

sf::Sprite & Animation::getSprite()
{
    return m_sprite;
}

int Animation::getCurrentAnimationFrameIndex() const
{
    if (m_speed == 0)
    {
        return 0;
    }

    // zero-indexed frame count
    int fullAnimationFramesPlayed = (int) floor(m_currentFrame / m_speed);
    // zero-indexed animation frames
    int currentAnimationFrame = fullAnimationFramesPlayed % m_frameCount;

    return currentAnimationFrame;
}

void Animation::setCurrentAnimationFrame(int index)
{
    m_currentFrame = index * m_speed;
}