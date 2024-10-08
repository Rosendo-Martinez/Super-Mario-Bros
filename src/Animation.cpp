#include "Animation.h"
#include <cmath>
#include <cassert>
#include <iostream>

// Should not be used
Animation::Animation()
{
}

// For single-frame textures
Animation::Animation(const std::string & name, const sf::Texture & t)
    : Animation(name, t, 1, 0)
{
}

// For single-frame textures with duration
// Duration is essentially the same as speed
Animation::Animation(const std::string & name, const sf::Texture & t, size_t duration)
    : Animation(name, t, 1, duration)
{
}

// For single-frame or multi-frame textures
Animation::Animation(const std::string & name, const sf::Texture & t, size_t frameCount, size_t speed)
    : Animation(name, t, frameCount, speed, 1.f, 1.f)
{
}

// For single-frame or multi-frame textures
Animation::Animation(const std::string & name, const sf::Texture & t, size_t frameCount, size_t speed, float scaleX, float scaleY)
    : Animation(name, t, frameCount, speed, scaleX, scaleY, -1, -1)
{
}

/**
 * If ox or oy are -1, then the origin will placed on the center of the first frame.
 */
Animation::Animation(const std::string & name, const sf::Texture & t, size_t frameCount, size_t speed, float scaleX, float scaleY, float ox, float oy)
    : m_name(name)
    , m_sprite(t)
    , m_frameCount(frameCount)
    , m_currentFrame(0)
    , m_speed(speed)
{
    assert(frameCount > 1 ? (speed > 0) : true); // Speed must be non-zero for multi-frame assets
    m_size = Vec2((float)t.getSize().x / frameCount, (float)t.getSize().y);
    if (ox == -1 || oy == -1)
    {
        m_sprite.setOrigin(m_size.x / 2.0f, m_size.y / 2.0f);
    }
    else
    {
        std::cout << "Custom Origin " << ox << " " << oy << "\n";
        m_sprite.setOrigin(ox,oy);
    }
    m_sprite.setTextureRect(sf::IntRect(0, 0, m_size.x, m_size.y));
    m_sprite.setScale(sf::Vector2f(scaleX, scaleY));
}

// Call once per frame.
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
    if (m_speed == 0 || floor(m_currentFrame / m_speed) >= (double) m_frameCount)
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

/*
Returns the index of the texture frame that the animation
is currently on.
*/
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

/*
Sets the current texture frame the animation is on.

This is what it does:
currentFrame = index * speed

Take this into account when using this method.
*/
void Animation::setCurrentAnimationFrame(int index)
{
    m_currentFrame = index * m_speed;
}