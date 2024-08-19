#pragma once

#include <SFML/Graphics.hpp>
#include <string>

#include "Vec2.h"

class Animation
{
private:
    sf::Sprite   m_sprite;
    int          m_frameCount     = 0;
    int          m_currentFrame   = 0;
    int          m_speed          = 0;
    Vec2         m_size           = { 0.0, 0.0 };
    std::string  m_name           = "";
public:
    Animation();
    Animation(const std::string & name, const sf::Texture & t);
    Animation(const std::string & name, const sf::Texture & t, size_t frameCount, size_t speed);
    Animation(const std::string & name, const sf::Texture & t, size_t frameCount, size_t speed, float scaleX, float scaleY);
    void update();
    bool hasEnded() const;
    const std::string & getName() const;
    const Vec2 & getSize() const;
    sf::Sprite & getSprite();
};
