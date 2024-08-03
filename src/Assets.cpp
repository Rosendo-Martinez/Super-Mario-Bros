#include "Assets.h"
#include <iostream>
#include <cassert>

Assets::Assets()
{
}

void Assets::addTexture(const std::string & name, const std::string & path)
{
    sf::Texture texture;
    bool result = texture.loadFromFile(path);
    assert(result && "Failed to load texture");
    
    m_textures[name] = texture;
}

void Assets::addAnimation(const std::string & name, const Animation & animation)
{
    m_animations[name] = animation;
}

void Assets::addSound(const std::string & name, const std::string & path)
{
}

void Assets::addFont(const std::string & name, const std::string & path)
{
    sf::Font font;
    bool result = font.loadFromFile(path);
    assert(result && "Failed to load font");

    m_fonts[name] = font;
}

const sf::Texture & Assets::getTexture(const std::string & name) const
{
    std::cout << name << '\n';
    assert(m_textures.find(name) != m_textures.end() && "Key is wrong or texture does not exist.");

    return m_textures.at(name);
}

const Animation & Assets::getAnimation(const std::string & name) const
{
    assert(m_animations.find(name) != m_animations.end() && "Key is wrong or animation does not exist.");

    return m_animations.at(name);
}

const sf::Sound & Assets::getSound(const std::string & name) const
{
}

const sf::Font & Assets::getFont(const std::string & name) const
{
    assert(m_animations.find(name) != m_animations.end() && "Key is wrong or font does not exist.");

    return m_fonts.at(name);
}
