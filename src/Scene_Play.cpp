#include "Scene_Play.h"
#include <iostream> // for debuging only

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

    int height = window.getSize().y;
    int width = window.getSize().x;

    int verticalLines = height / 64;
    for (int i = 0; i < verticalLines; i++) 
    {
        int x = 64 * (i + 1);

        sf::VertexArray line(sf::Lines, 2);

        line[0].position = sf::Vector2f(x, 0);
        line[1].position = sf::Vector2f(x, height);

        line[0].color = sf::Color::White;
        line[1].color = sf::Color::White;

        window.draw(line);
    }

    int horizontalLines = width / 64;
    for (int i = 0; i < horizontalLines; i++)
    {
        int y = height - 64 * (i + 1);

        sf::VertexArray line(sf::Lines, 2);

        line[0].position = sf::Vector2f(0, y);
        line[1].position = sf::Vector2f(width, y);

        window.draw(line);
    }

    window.display();
}

void Scene_Play::onEnd() // before scene ends change to menu scene
{
}