#pragma once

#include "Scene.h"
#include "Assets.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <map>
#include <memory>

class GameEngine
{
private:
    std::map<std::string, std::shared_ptr<Scene>> m_scenes;
    sf::RenderWindow m_window;
    Assets m_assets;
    std::string m_currentScene;
    bool running;

    void init();
    std::shared_ptr<Scene> currentScene();
public:
    void run();
    void update();
    void quit();
    void changeScene(std::string name, std::shared_ptr<Scene> scene); // changes scene to new or existing scene
    Assets & getAssets();
    sf::Window & window();
    void sUserInput();
};