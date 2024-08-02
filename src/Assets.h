#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <map>
#include <string>
#include "Animation.h"

class Assets
{
private:
    std::map<std::string, sf::Texture> m_textures;
    std::map<std::string, Animation> m_animations;
    std::map<std::string, sf::Sound> m_sounds;
    std::map<std::string, sf::Font> m_fonts;
public:
    Assets();

    void addTexture(const std::string & name, const std::string & path);
    void addAnimation(const std::string & name, const Animation & animation);
    void addSound(std::string name, std::string path);
    void addFont(std::string name, std::string path);

    sf::Texture & getTexture(const std::string & name);
    Animation & getAnimation(const std::string & name);
    sf::Sound & getSound(std::string name);
    sf::Font & getFont(std::string name);
};