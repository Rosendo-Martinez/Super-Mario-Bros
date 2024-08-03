#include "GameEngine.h"
#include "Scene_Play.h"
#include <iostream>

GameEngine::GameEngine()
{
    init(""); 
}

void GameEngine::init(const std::string & assetSpecFilePath) // load in all assets, create window, frame limit, set menu scene
{
    m_window.create(sf::VideoMode(64*10,64*10), "Super Mario World");
    m_window.setKeyRepeatEnabled(false);
    
    m_assets.addFont("Grid", "bin/fonts/Roboto-Regular.ttf");

    changeScene("Scene_Play", std::make_shared<Scene_Play>(this, assetSpecFilePath), true);
}

void GameEngine::update()
{
}

void GameEngine::sUserInput() // get user input, and pass it to scene as action if scene has it registered
{
    sf::Event e;
    while (m_window.pollEvent(e))
    {
        if (e.type == sf::Event::KeyPressed)
        {
            if (e.key.code == sf::Keyboard::G)
            {
                m_sceneMap[m_currentScene]->sDoAction(Action("TOGGLE_GRID", "START"));
            }
        }

        if (e.type == sf::Event::KeyReleased)
        {
        }
    }
}

std::shared_ptr<Scene> GameEngine::currentScene()
{
}

GameEngine::GameEngine(const std::string & assetSpecFilePath)
{
}

void GameEngine::changeScene(const std::string & sceneName, std::shared_ptr<Scene> scene, bool endCurrentScene)
{
    m_sceneMap[sceneName] = scene;

    if (endCurrentScene)
    {
        m_currentScene = sceneName;
    }
}

void GameEngine::quit() // closes the game
{
}

void GameEngine::run() // main game loop
{
    while (m_window.isOpen())
    {
        sUserInput();
        m_sceneMap[m_currentScene]->update();
        m_sceneMap[m_currentScene]->sRender();
    }
}

sf::RenderWindow & GameEngine::window()
{
    return m_window;
}

const Assets & GameEngine::assets() const
{
    return m_assets;
}

bool GameEngine::isRunning()
{
}