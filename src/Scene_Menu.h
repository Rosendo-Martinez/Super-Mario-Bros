#pragma once

#include "Scene.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "Action.h"

class Scene_Menu : public Scene
{
private:
    std::vector<std::string> m_menuStrings;
    sf::Text m_menuText;
    std::vector<std::string> m_levelPaths;
    int menuIndex;

    void init();
    void sRender();
    void sDoAction();
public:
    void update();
};