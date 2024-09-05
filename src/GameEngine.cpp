#include "GameEngine.h"
#include "Scene_Play.h"
#include <iostream>
#include <fstream>

GameEngine::GameEngine()
{
    init(""); 
}

void GameEngine::init(const std::string & assetSpecFilePath) // load in all assets, create window, frame limit, set menu scene
{
    m_window.create(sf::VideoMode(64*10,64*10), "Super Mario World");
    m_window.setKeyRepeatEnabled(false);
    m_window.setFramerateLimit(60);

    std::ifstream assetsFile ("bin/texts/assets.txt");
        
    if (!assetsFile.is_open())
    {
        std::cout << "Error: could not open assets file.\n";
    }
    else
    {
        while (!assetsFile.eof())
        {
            std::string type;
            assetsFile >> type;

            if (type == "Texture")
            {
                std::string name;
                std::string path;

                assetsFile >> name >> path;

                m_assets.addTexture(name, path);
            }
            else if (type == "Animation")
            {
                std::string name;
                std::string textureName;
                int frameCount;
                int speed;

                assetsFile >> name >> textureName >> frameCount >> speed;

                m_assets.addAnimation(name, Animation(name, m_assets.getTexture(textureName), frameCount, speed));
            }
            else if (type == "Font")
            {
                std::string name;
                std::string path;

                assetsFile >> name >> path;

                m_assets.addFont(name, path);
            }
            else
            {
                std::cout << "Error: " << type << "is not a supported asset type.\n";
            }
        }
    }

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
        const ActionMap & actions = m_sceneMap[m_currentScene]->getActionMap();
        if (e.type == sf::Event::KeyPressed)
        {
            if (actions.count(e.key.code) == 1)
            {
                m_sceneMap[m_currentScene]->sDoAction(Action(actions.at(e.key.code), "START"));
            }
        }
        else if (e.type == sf::Event::KeyReleased)
        {
            if (actions.count(e.key.code) == 1)
            {
                m_sceneMap[m_currentScene]->sDoAction(Action(actions.at(e.key.code), "END"));
            }
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