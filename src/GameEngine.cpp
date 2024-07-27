#include "GameEngine.h"

GameEngine::GameEngine()
{
}

void GameEngine::init(const std::string & assetSpecFilePath) // load in all assets, create window, frame limit, set menu scene
{
}

void GameEngine::update()
{
}

void GameEngine::sUserInput() // get user input, and pass it to scene as action if scene has it registered
{
}

std::shared_ptr<Scene> GameEngine::currentScene()
{
}

GameEngine::GameEngine(const std::string & assetSpecFilePath)
{
}

void GameEngine::changeScene(const std::string & sceneName, std::shared_ptr<Scene> scene, bool endCurrentScene) // changes scene to new or existing scene 
{
}

void GameEngine::quit() // closes the game
{
}

void GameEngine::run() // main game loop
{
}

sf::RenderWindow & GameEngine::window()
{
}

const Assets & GameEngine::assets() const
{
}

bool GameEngine::isRunning()
{
}