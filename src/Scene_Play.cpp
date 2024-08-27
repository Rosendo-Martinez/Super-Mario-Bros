#include "Scene_Play.h"
#include "Physics.h"
#include <iostream>
#include <sstream> 
#include <cmath>
#include <fstream>

void Scene_Play::init(const std::string & levelPath) // register actions, font/text, loadlevel(path)
{
    spawnPlayer();
    loadLevel(levelPath);
    registerAction(sf::Keyboard::G, "TOGGLE_GRID");
    registerAction(sf::Keyboard::C, "TOGGLE_BOUNDING_BOXES");
    registerAction(sf::Keyboard::T, "TOGGLE_TEXTURES");
    registerAction(sf::Keyboard::W, "UP");
    registerAction(sf::Keyboard::S, "DOWN");
    registerAction(sf::Keyboard::A, "LEFT");
    registerAction(sf::Keyboard::D, "RIGHT");
}

Vec2 Scene_Play::gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity)
{
    sf::RenderWindow & window = m_game->window();

    sf::Sprite sprite = entity->getComponent<CAnimation>().animation.getSprite();
    const int heighGrid = window.getSize().y;
    const int widthGrid = window.getSize().x;
    const int heightCell = m_gridSize.y;
    const int widthCell = m_gridSize.x;

    // grid to cartesian coordinates
    float x_c = widthCell * (gridX);
    float y_c = heighGrid - heightCell * (gridY);

    // center of entity
    float x_m = x_c + sprite.getGlobalBounds().width / 2;
    float y_m = y_c - sprite.getGlobalBounds().height / 2;

    return Vec2(x_m, y_m);
}

void Scene_Play::loadLevel(const std::string & filename) // load/reset/reload level
{
    std::ifstream levelSpec ("bin/texts/level1.txt");

    if (!levelSpec.is_open())
    {
        std::cout << "Error: level specification file could not be open.\n";
    }
    else
    {
        while (levelSpec)
        {
            std::string type;
            levelSpec >> type;

            if (type == "Tile")
            {
                auto e = m_entityManager.addEntity(type);
                std::string animationName;
                float gx;
                float gy;

                levelSpec >> animationName >> gx >> gy;

                e->addComponent<CAnimation>(m_game->assets().getAnimation(animationName), true);
                e->addComponent<CTransform>(gridToMidPixel(gx,gy,e));
                e->addComponent<CBoundingBox>(Vec2(64,64));
            }
            else if (type == "Decoration")
            {
                auto e = m_entityManager.addEntity(type);
                std::string animationName;
                float gx;
                float gy;

                levelSpec >> animationName >> gx >> gy;

                e->addComponent<CAnimation>(m_game->assets().getAnimation(animationName), true);
                e->addComponent<CTransform>(gridToMidPixel(gx,gy,e));
            }
            else if (type == "Player")
            {
                std::cout << type << "\n";
            }
            else
            {
                std::cout << "Error: " << type << " is not a supported entity type.\n";
            }
        }
    }

    std::cout << "Entities Created: " << m_entityManager.getTotalEntitiesCreated() << "\n";
}

void Scene_Play::spawnPlayer()
{
    auto player = m_entityManager.addEntity("Player");
    player->addComponent<CAnimation>(m_game->assets().getAnimation("MarioStand"), true);
    player->addComponent<CTransform>(gridToMidPixel(4,7,player));
    player->addComponent<CBoundingBox>(Vec2(64, 64));
    player->addComponent<CInput>();
    player->addComponent<CState>();
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

    init("");
}

void Scene_Play::update() // update EM, and cal systems
{
    m_entityManager.update();

    std::cout << m_player->getComponent<CState>().state << "\n";

    sAnimation();
    sMovement();
    sCollision();
    sRender();
}

void Scene_Play::sAnimation()
{
    // if player is running
        // give him running animation
    // if player is standing
        // give him standing animation
    // if player is jumping
        // give him jumping animation
    
    // if player is pressing left
        // flip animation to left
    // if player is pressing right
        // flip animatoin to right

    if (m_player->getComponent<CState>().state == "Running" && m_player->getComponent<CAnimation>().animation.getName() != "MarioRun")
    {
        m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("MarioRun");
    }
    else if (m_player->getComponent<CState>().state == "Standing" && m_player->getComponent<CAnimation>().animation.getName() != "MarioStand")
    {
        m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("MarioStand");
    }
    else if (m_player->getComponent<CState>().state == "Jumping" && m_player->getComponent<CAnimation>().animation.getName() != "MarioAir")
    {
        m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("MarioAir");
    }

    for (auto e: m_entityManager.getEntities())
    {
        e->getComponent<CAnimation>().animation.update();
    }
}

void Scene_Play::sMovement()
{
    float speed = 5.f;
    float initialJumpSpeed = 24.f;
    float gravity = 1.f;
    m_player->getComponent<CTransform>().velocity = Vec2(0,m_player->getComponent<CTransform>().velocity.y + gravity);


    if (m_player->getComponent<CInput>().left)
    {
        m_player->getComponent<CTransform>().velocity.x -= speed;
    }

    if (m_player->getComponent<CInput>().right)
    {
        m_player->getComponent<CTransform>().velocity.x += speed;
    }

    if (m_player->getComponent<CInput>().up && (m_player->getComponent<CState>().state == "Standing" || m_player->getComponent<CState>().state == "Running"))
    {
        m_player->getComponent<CTransform>().velocity.y -= initialJumpSpeed;
        m_player->getComponent<CState>().state = "Jumping";
    }

    if (m_player->getComponent<CInput>().down)
    {
        m_player->getComponent<CTransform>().velocity.y += speed;
    }

    if (!m_player->getComponent<CInput>().up && m_player->getComponent<CState>().state == "Jumping" && m_player->getComponent<CTransform>().velocity.y < 0)
    {
        m_player->getComponent<CTransform>().velocity.y = 0;
    }

    for (auto e : m_entityManager.getEntities())
    {
        if (e->hasComponent<CTransform>())
        {
            e->getComponent<CTransform>().prevPos = e->getComponent<CTransform>().pos;
            e->getComponent<CTransform>().pos += e->getComponent<CTransform>().velocity;
        }
    }
}

void Scene_Play::sEnemySpawn()
{
}

void Scene_Play::sCollision()
{
    int collisionCount = 0;
    // player-block collisions
    for (auto e : m_entityManager.getEntities())
    {
        if (m_player->id() == e->id() || !e->hasComponent<CBoundingBox>())
        {
            continue;
        }
        
        Vec2 overlap = Physics::GetOverlap(m_player, e);
        Vec2 prevOverlap = Physics::GetPreviousOverlap(m_player, e);

        // collision
        if (overlap.x > 0 && overlap.y > 0)
        {
            // horizontal collision
            if (prevOverlap.y > 0)
            {
                // came from left
                if (m_player->getComponent<CTransform>().prevPos.x < e->getComponent<CTransform>().prevPos.x)
                {
                    // push left
                    m_player->getComponent<CTransform>().pos.x -= overlap.x;
                }
                // came from right
                else
                {
                    // push right
                    m_player->getComponent<CTransform>().pos.x += overlap.x;
                }
            }
            // vertical collision
            else if (prevOverlap.x > 0)
            {
                std::cout << "Vertical Collision.\n";
                // came from top
                if (m_player->getComponent<CTransform>().prevPos.y < e->getComponent<CTransform>().prevPos.y)
                {
                    // push up
                    m_player->getComponent<CTransform>().pos.y -= overlap.y;
                    
                    if (m_player->getComponent<CInput>().left || m_player->getComponent<CInput>().right)
                    {
                        m_player->getComponent<CState>().state = "Running";
                    }
                    else
                    {
                        m_player->getComponent<CState>().state = "Standing";
                    }
                }
                // came from bottom
                else
                {
                    // push down
                    m_player->getComponent<CTransform>().pos.y += overlap.y;
                }

                m_player->getComponent<CTransform>().velocity.y = 0;
            }
            // diagonal collision 
            else
            {
                // came from top
                if (m_player->getComponent<CTransform>().prevPos.y < e->getComponent<CTransform>().prevPos.y)
                {
                    // push up
                    m_player->getComponent<CTransform>().pos.y -= overlap.y;

                    if (m_player->getComponent<CInput>().left || m_player->getComponent<CInput>().right)
                    {
                        m_player->getComponent<CState>().state = "Running";
                    }
                    else
                    {
                        m_player->getComponent<CState>().state = "Standing";
                    }
                }
                // came from bottom
                else
                {
                    // push down
                    m_player->getComponent<CTransform>().pos.y += overlap.y;
                }

                m_player->getComponent<CTransform>().velocity.y = 0;
            }

            collisionCount++;
        }
    }

    if (collisionCount == 0)
    {
        m_player->getComponent<CState>().state = "Jumping";
    }
}

void Scene_Play::sRender()
{
    sf::RenderWindow & window = m_game->window();
    window.clear(sf::Color(97, 126, 248)); 

    // Draw entities
    if (m_drawTextures)
    {
        for (auto e : m_entityManager.getEntities("Decoration"))
        {
            Vec2 pos = e->getComponent<CTransform>().pos;
            sf::Sprite & sprite = e->getComponent<CAnimation>().animation.getSprite();

            sprite.setPosition(sf::Vector2f(pos.x,pos.y));
            window.draw(sprite);
        }

        for (auto e : m_entityManager.getEntities("Tile"))
        {
            Vec2 pos = e->getComponent<CTransform>().pos;
            sf::Sprite & sprite = e->getComponent<CAnimation>().animation.getSprite();

            sprite.setPosition(sf::Vector2f(pos.x,pos.y));
            window.draw(sprite);
        }

        for (auto e : m_entityManager.getEntities("Player"))
        {
            Vec2 pos = e->getComponent<CTransform>().pos;
            sf::Sprite & sprite = e->getComponent<CAnimation>().animation.getSprite();

            sprite.setPosition(sf::Vector2f(pos.x,pos.y));
            window.draw(sprite);
        }
    }

    // Draw Bounding Boxes
    if (m_drawCollision)
    {
        for (auto e : m_entityManager.getEntities())
        {
            Vec2 pos = e->getComponent<CTransform>().pos;
            Vec2 size = e->getComponent<CBoundingBox>().size;
            sf::RectangleShape bb(sf::Vector2f(size.x,size.y));

            bb.setPosition(sf::Vector2f(pos.x - size.x/2, pos.y - size.y/2));
            bb.setFillColor(sf::Color(0,0,0,0));
            bb.setOutlineColor(sf::Color::White);
            bb.setOutlineThickness(1.f);

            window.draw(bb);
        }
    }

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

        if (action.name() == "TOGGLE_BOUNDING_BOXES")
        {
            m_drawCollision = !m_drawCollision;
        }

        if (action.name() == "TOGGLE_TEXTURES")
        {
            m_drawTextures = !m_drawTextures;
        }

        if (action.name() == "UP")
        {
            m_player->getComponent<CInput>().up = true;
        }

        if (action.name() == "DOWN")
        {
            m_player->getComponent<CInput>().down = true;
        }

        if (action.name() == "LEFT")
        {
            m_player->getComponent<CInput>().left = true;
            std::cout << "Action Move Left\n";
        }

        if (action.name() == "RIGHT")
        {
            m_player->getComponent<CInput>().right = true;
        }
    }
    else if (action.type() == "END")
    {
        if (action.name() == "UP")
        {
            m_player->getComponent<CInput>().up = false;
        }

        if (action.name() == "DOWN")
        {
            m_player->getComponent<CInput>().down = false;
        }

        if (action.name() == "LEFT")
        {
            m_player->getComponent<CInput>().left = false;
        }

        if (action.name() == "RIGHT")
        {
            m_player->getComponent<CInput>().right = false;
        }
    }
}

void Scene_Play::sDebug()
{   
}

void Scene_Play::onEnd() // before scene ends change to menu scene
{
}