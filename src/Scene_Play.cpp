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
    auto player = m_entityManager.addEntity("Player");
    player->addComponent<CTransform>(Vec2(64*3, 64*3));
    player->addComponent<CAnimation>(m_game->assets().getAnimation("MarioStand"), true);
    m_player = player;
}

void Scene_Play::spawnBullet(std::shared_ptr<Entity> entity)
{
}

Scene_Play::Scene_Play(GameEngine * gameEngine, const std::string & levelPath)
    : Scene(gameEngine)
{
    m_gridText.setFont(m_game->assets().getFont("Grid"));
    m_gridText.setCharacterSize(12);
    m_gridText.setFillColor(sf::Color::White);

    registerAction(sf::Keyboard::G, "TOGGLE_GRID");
    spawnPlayer();
}

void Scene_Play::update() // update EM, and cal systems
{
    sRender();
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
    sf::RenderWindow & window = m_game->window();
    window.clear(sf::Color::Blue); 

    // Draw the player
    Vec2 playerPos = m_player->getComponent<CTransform>().pos;
    sf::Sprite playerSprite = m_player->getComponent<CAnimation>().animation.getSprite();
    playerSprite.setPosition(sf::Vector2f(playerPos.x, playerPos.y));
    playerSprite.scale(sf::Vector2f(4.f,4.f));
    window.draw(playerSprite);

    if (m_drawGrid)
    {
        const int heightWindow = window.getSize().y;
        const int widthWindow = window.getSize().x;
        const int heightCell = m_gridSize.y;
        const int widthCell = m_gridSize.x;


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

    }

    window.display();
}

void Scene_Play::sDoAction(const Action & action) // do the action
{
    if (action.type() == "START")
    {
        if (action.name() == "TOGGLE_GRID")
        {
            m_drawGrid = !m_drawGrid;
        }
    }
    else if (action.type() == "END")
    {

    }
}

void Scene_Play::sDebug()
{   
}

void Scene_Play::onEnd() // before scene ends change to menu scene
{
}