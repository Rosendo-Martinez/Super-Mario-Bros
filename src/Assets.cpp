#include "Assets.h"
#include <iostream>

Assets::Assets()
{
}

void Assets::addTexture(const std::string & name, const std::string & path)
{
    sf::Texture texture;
    if (!texture.loadFromFile(path))
    {
        std::cout << "Error: could not load " << name << " texture with path " << path << std::endl;
        return;
    }
    
    m_textures[name] = texture;
}

void Assets::addAnimation(const std::string & name, const Animation & animation)
{
    m_animations[name] = animation;
}

void Assets::addSound(std::string name, std::string path)
{
}

void Assets::addFont(std::string name, std::string path)
{
}

sf::Texture & Assets::getTexture(const std::string & name)
{
    if (m_textures.find(name) == m_textures.end())
    {
        std::cout << "Error: called getTexture() with name " << name <<  ", but texture does not exist!\n";
    }

    return m_textures[name];
}

Animation & Assets::getAnimation(const std::string & name)
{
    if (m_animations.find(name) == m_animations.end())
    {
        std::cout << "Error: called getAnimation() with name " << name << ", but animation does not exist!\n";
    }

    return m_animations[name];
}

sf::Sound & Assets::getSound(std::string name)
{
}

sf::Font & Assets::getFont(std::string name)
{
}
