#pragma once

#include "Scene.h"
#include "Assets.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <map>
#include <memory>

class Scene;

typedef std::map<std::string, std::shared_ptr<Scene>> SceneMap;

class GameEngine
{
protected:
    sf::RenderWindow m_window;
    Assets m_assets;
    std::string m_currentScene;
    SceneMap m_sceneMap;
    size_t m_simulationSpeed = 1;
    bool m_running = true;

    void init(const std::string & assetSpecFilePath); // load in all assets, create window, frame limit, set menu scene
    void update();

    void sUserInput(); // get user input, and pass it to scene as action if scene has it registered

    std::shared_ptr<Scene> currentScene();
public:
    GameEngine();
    GameEngine(const std::string & assetSpecFilePath);

    void changeScene(const std::string & sceneName, std::shared_ptr<Scene> scene, bool endCurrentScene = false); // changes scene to new or existing scene 

    void quit(); // closes the game
    void run(); // main game loop

    sf::RenderWindow & window();
    const Assets & assets() const;
    bool isRunning();
};