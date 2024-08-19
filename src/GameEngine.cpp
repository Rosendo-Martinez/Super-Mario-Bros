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
    m_window.setFramerateLimit(60);
    
    m_assets.addFont("Grid", "bin/fonts/Roboto-Regular.ttf");
    m_assets.addTexture("MarioStand", "bin/images/mario/MarioStand16.png");
    m_assets.addTexture("QuestionBlock", "bin/images/mario/QuestionMarkBlock16.png");
    m_assets.addTexture("Brick", "bin/images/mario/Brick16.png");
    m_assets.addTexture("QuestionMarkBlink", "bin/images/mario/QuestionMarkBlink16.png");

    m_assets.addAnimation("MarioStand", Animation("MarioStand", m_assets.getTexture("MarioStand"), 1, 0, 4.f, 4.f));
    m_assets.addAnimation("QuestionBlock", Animation("QuestionBlock", m_assets.getTexture("QuestionBlock"), 1, 0, 4.f, 4.f));
    m_assets.addAnimation("Brick", Animation("Brick", m_assets.getTexture("Brick"), 1, 0, 4.f, 4.f));
    m_assets.addAnimation("QuestionMarkBlink", Animation("QuestionMarkBlink", m_assets.getTexture("QuestionMarkBlink"), 3, 5, 4.f, 4.f));

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
            const ActionMap & actions = m_sceneMap[m_currentScene]->getActionMap();

            if (actions.count(e.key.code) == 1)
            {
                m_sceneMap[m_currentScene]->sDoAction(Action(actions.at(e.key.code), "START"));
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