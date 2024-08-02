#include "Scene_Play.h"
#include <iostream> // for debuging only
#include <cmath>

void Scene_Play::init(const std::string & levelPath) // register actions, font/text, loadlevel(path)
{
}

Vec2 Scene_Play::gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity)
{
}

void Scene_Play::loadLevel(const std::string & filename) // load/reset/reload level
{
}

void Scene_Play::spawnPlayer()
{
}

void Scene_Play::spawnBullet(std::shared_ptr<Entity> entity)
{
}

Scene_Play::Scene_Play(GameEngine * gameEngine, const std::string & levelPath)
    : Scene(gameEngine)
{
}

void Scene_Play::update() // update EM, and cal systems
{
    sDebug();
}

void Scene_Play::sAnimation()
{
}

void Scene_Play::sMovement()
{
}

void Scene_Play::sEnemySpawn()
{
}

void Scene_Play::sCollision()
{
}

void Scene_Play::sRender()
{
}

void Scene_Play::sDoAction(const Action & action) // do the action
{
}

void Scene_Play::sDebug()
{   
    sf::RenderWindow & window = m_game->window();
    window.clear(sf::Color::Blue); 

    int heightWindow = window.getSize().y;
    int widthWindow = window.getSize().x;
    int heightCell = m_gridSize.y;
    int widthCell = m_gridSize.x;

    // Draw grid vertical lines
    int verticalLines = floor(widthWindow / widthCell);
    for (int i = 0; i < verticalLines; i++) 
    {
        int x = widthCell * (i + 1);

        sf::VertexArray line(sf::Lines, 2);

        line[0].position = sf::Vector2f(x, 0);
        line[1].position = sf::Vector2f(x, heightWindow);

        line[0].color = sf::Color::White;
        line[1].color = sf::Color::White;

        window.draw(line);
    }

    // Draw grid horizontal lines
    int horizontalLines = floor(heightWindow / heightCell);
    for (int i = 0; i < horizontalLines; i++)
    {
        int y = heightWindow - heightCell * (i + 1);

        sf::VertexArray line(sf::Lines, 2);

        line[0].position = sf::Vector2f(0, y);
        line[1].position = sf::Vector2f(widthWindow, y);

        window.draw(line);
    }

    window.display();
}

void Scene_Play::onEnd() // before scene ends change to menu scene
{
}