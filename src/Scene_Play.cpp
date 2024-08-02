#include "Scene_Play.h"
#include <iostream>
#include <sstream> 
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
    if (!m_gridFont.loadFromFile("bin/fonts/Roboto-Regular.ttf"))
    {
        std::cout << "Error: could not load grid font!\n";
    }

    m_gridText.setFont(m_gridFont);
    m_gridText.setCharacterSize(12);
    m_gridText.setFillColor(sf::Color::White);
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
    const int heightWindow = window.getSize().y;
    const int widthWindow = window.getSize().x;
    const int heightCell = m_gridSize.y;
    const int widthCell = m_gridSize.x;

    window.clear(sf::Color::Blue); 

    // Draw grid vertical lines
    const int verticalLines = ceil((float) widthWindow / widthCell);
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
    const int horizontalLines = ceil((float) heightWindow / heightCell);
    for (int i = 0; i < horizontalLines; i++)
    {
        int y = heightWindow - heightCell * (i + 1);
        sf::VertexArray line(sf::Lines, 2);

        line[0].position = sf::Vector2f(0, y);
        line[1].position = sf::Vector2f(widthWindow, y);
        line[0].color = sf::Color::White;
        line[1].color = sf::Color::White;

        window.draw(line);
    }

    // Draw grid coordinates
    for (int gx = 0; gx < verticalLines; gx++)
    {
        for (int gy = 0; gy < horizontalLines; gy++)
        {
            std::ostringstream oss;
            oss << "(" << gx << "," << gy << ")";
            m_gridText.setString(oss.str());

            // relative to top left of window
            int x = widthCell * gx;
            int y = heightWindow - (heightCell * (gy + 1));
            m_gridText.setPosition(sf::Vector2f(x, y));
            
            window.draw(m_gridText);
        }
    }

    window.display();
}

void Scene_Play::onEnd() // before scene ends change to menu scene
{
}