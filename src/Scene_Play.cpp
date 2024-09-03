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

    if (!m_player->isActive())
    {
        spawnPlayer();
    }

    sAnimation();
    sMovement();
    sCollision();
    sRender();

    std::cout << "Entity Count: " << m_entityManager.getEntities().size() << "\n";
}

void Scene_Play::sAnimation()
{
    // Set Mario's animation to match his current state
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
    float speed = 6.f;
    float initialJumpSpeed = 24.f;
    float gravity = 1.f;
    m_player->getComponent<CTransform>().velocity = Vec2(0,m_player->getComponent<CTransform>().velocity.y + gravity);

    // Player run left
    if (m_player->getComponent<CInput>().left)
    {
        m_player->getComponent<CTransform>().velocity.x -= speed;
        
        // make player face left
        if (m_player->getComponent<CTransform>().scale.x > 0)
        {
            m_player->getComponent<CTransform>().scale.x *= -1;
        }
    }

    // Player run right
    if (m_player->getComponent<CInput>().right)
    {
        m_player->getComponent<CTransform>().velocity.x += speed;

        // make player face right
        if (m_player->getComponent<CTransform>().scale.x < 0)
        {
            m_player->getComponent<CTransform>().scale.x *= -1;   
        }
    }

    // Player jump
    if (m_player->getComponent<CInput>().up && m_player->getComponent<CInput>().canJump)
    {
        m_player->getComponent<CTransform>().velocity.y -= initialJumpSpeed;
        m_player->getComponent<CState>().state = "Jumping";
        m_player->getComponent<CInput>().canJump = false;
    }

    // Player move down
    if (m_player->getComponent<CInput>().down)
    {
        m_player->getComponent<CTransform>().velocity.y += speed;
    }

    // Player jump strength
    if (!m_player->getComponent<CInput>().up && m_player->getComponent<CState>().state == "Jumping" && m_player->getComponent<CTransform>().velocity.y < 0)
    {
        m_player->getComponent<CTransform>().velocity.y = 0;
    }

    // Use velocity to move the entities positions
    for (auto e : m_entityManager.getEntities())
    {
        if (e->hasComponent<CTransform>())
        {
            e->getComponent<CTransform>().prevPos = e->getComponent<CTransform>().pos;
            e->getComponent<CTransform>().pos += e->getComponent<CTransform>().velocity;
        }

        // Entity fell down hole
        if (e->getComponent<CTransform>().pos.y + 64/2 > m_game->window().getSize().y)
        {
            e->destroy();
        }
    }

    // ----------------------------------------------------------------------

    // Only for player-block collisions:
    // Basic idea behind collisions is that the player only needs to resolve collisions for at most one block in each direction.
    // For example, if the player collides with 2 blocks and both collisions are from the left, then the player really only needs to do
    // collisions resolution for one since fixing one should also fix the other.
    // (Note: mario can only ever be colliding with 1 block in any diagonal direction.)

    // upColDirBlock = null
    // ...

    // FIND BLOCKS THAT MARIO COLLIDES WITH
    // for each block in Tiles
        // if player collides with block
            // collisionDir = getCollisionDirection(player, block)

            // if collisionDir == up
                // if upColDirBlock == null OR current block has greater x-overlap:
                    // upColDirBlock = block
            // if collisionDir == left
                // if leftColDirBlock == null OR current block is higher up from the ground:
                    // leftColDirBlock = block
            // if collisionDir == right
                // if rightColDirBlock == null OR current block is higher up from the ground:
                    // rightColDirBlock = block
            // if collisionDir == down
                // if downColDirBlock == null OR current block has a greater x-overlap:
                    // downColDirBlock = block

            // if collisionDir == diagonalTL
                // diagonalTLColDirBlock = block
            // if collisionDir == diagonalTR
                // diagonalTRColDirBlock = block
            // if collisionDir == diagonalBL
                // diagonalBLColDirBlock = block
            // if collisionDir == diagonalBR
                // diagonalBRColDirBlock = block

    // Do CR for up collision.
    // if upColDirBlock != null
        // push player down
        // give player zero y-dir velocity

        // Handle CR for special blocks:
            // ? block
            // Brick block

    // Do CR for left collision.
    // if leftColDirBlock != null
        // Todo: figure out pull up mechanic for mario.

        // push left

        // No special blocks to do CR for.
    
    // Do CR for right collision.
    // if rightColDirBlock != null
        // Todo: figure out pull up mechanic for mario.

        // push right

        // No special blocks to do CR for.

    // Do CR for down collision.
    // if downColDirBlock != null
        // push up

        // No special blocks to do CR for.
    
    // Do CR for diagonal TL collision.
    // if their was DTL col:
        // push left

    // Do CR for diagonal TR collision.
    // if their was DTR col:
        // push right

    // Do CR for diagonal BL collision.
    // if their was DBL col:
        // push left


    // Do CR for diagonal BR collision.
    // if their was DBR col:
        // push right
}

void Scene_Play::sEnemySpawn()
{
}

enum class CollisionDirection 
{
    DIAGONAL_TOP_LEFT,
    DIAGONAL_TOP_RIGHT,
    DIAGONAL_BOTTOM_LEFT,
    DIAGONAL_BOTTOM_RIGHT,
    TOP,
    BOTTOM,
    LEFT,
    RIGHT
};

// Returns the direction player came from relative to block.
CollisionDirection getCollisionDirection(Vec2 prevOverlap, Vec2 prevPosPlayer, Vec2 prevPosBlock)
{
    // Horizontal direction
    if (prevOverlap.y > 0) {
        // came from left
        if (prevPosPlayer.x < prevPosBlock.x)
        {
            return CollisionDirection::LEFT;
        }
        // came from right
        else
        {
            return CollisionDirection::RIGHT;
        }
    }
    // Vertical direction
    else if (prevOverlap.x > 0)
    {
        // came from top
        if (prevPosPlayer.y < prevPosBlock.y)
        {
            return CollisionDirection::TOP;
        }
        // came from bottom
        else
        {
            return CollisionDirection::BOTTOM;
        }
    }
    // Diagonal direction
    else
    {
        // came from top
        if (prevPosPlayer.y > prevPosBlock.y)
        {
            // came from left
            if (prevPosPlayer.x < prevPosBlock.x)
            {
                return CollisionDirection::DIAGONAL_TOP_LEFT;
            }
            // came from right
            else
            {
                return CollisionDirection::DIAGONAL_TOP_RIGHT;
            }
        }
        // came from bottom
        else
        {
            // came from left
            if (prevPosPlayer.x < prevPosBlock.x)
            {
                return CollisionDirection::DIAGONAL_BOTTOM_LEFT;
            }
            // came from right
            else
            {
                return CollisionDirection::DIAGONAL_BOTTOM_RIGHT;
            }   
        }
    }
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
            CollisionDirection cd = getCollisionDirection(prevOverlap, m_player->getComponent<CTransform>().prevPos, e->getComponent<CTransform>().prevPos);

            if (cd == CollisionDirection::LEFT)
            {
                // push left
                m_player->getComponent<CTransform>().pos.x -= overlap.x;
            }
            else if (cd == CollisionDirection::RIGHT)
            {
                // push right
                m_player->getComponent<CTransform>().pos.x += overlap.x;
            }
            else if (cd == CollisionDirection::TOP || cd == CollisionDirection::DIAGONAL_TOP_LEFT || cd == CollisionDirection::DIAGONAL_TOP_RIGHT)
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

                m_player->getComponent<CInput>().canJump = true;
                m_player->getComponent<CTransform>().velocity.y = 0;
            }
            else if (cd == CollisionDirection::BOTTOM || cd == CollisionDirection::DIAGONAL_BOTTOM_LEFT || cd == CollisionDirection::DIAGONAL_BOTTOM_RIGHT)
            {
                // push down
                m_player->getComponent<CTransform>().pos.y += overlap.y;
                m_player->getComponent<CTransform>().velocity.y = 0;
            }

            collisionCount++;
        }
    }

    if (collisionCount == 0)
    {
        m_player->getComponent<CState>().state = "Jumping";
        m_player->getComponent<CInput>().canJump = false;
    }
}

void Scene_Play::sRender()
{
    sf::RenderWindow & window = m_game->window();
    window.clear(sf::Color(97, 126, 248)); 

    // Make camera follow player
    m_cameraPosition.x = m_player->getComponent<CTransform>().pos.x - window.getSize().x/2;
    // Make sure camera does not go out of left bound
    if (m_cameraPosition.x < 0)
    {
        m_cameraPosition.x = 0;
    }
    // Make sure to render entities with positions relative to camera

    // Draw entities
    if (m_drawTextures)
    {
        // Why multiply scale by 4?
        // The assets are 16x16, but I prefer them to be scaled up to 64x64.
        // 16 * 4 = 64

        for (auto e : m_entityManager.getEntities("Decoration"))
        {
            Vec2 pos = e->getComponent<CTransform>().pos - m_cameraPosition;
            Vec2 scale = e->getComponent<CTransform>().scale;
            sf::Sprite & sprite = e->getComponent<CAnimation>().animation.getSprite();

            sprite.setPosition(sf::Vector2f(pos.x,pos.y));
            sprite.setScale(sf::Vector2f(scale.x * 4, scale.y * 4));
            window.draw(sprite);
        }

        for (auto e : m_entityManager.getEntities("Tile"))
        {
            Vec2 pos = e->getComponent<CTransform>().pos - m_cameraPosition;
            Vec2 scale = e->getComponent<CTransform>().scale;
            sf::Sprite & sprite = e->getComponent<CAnimation>().animation.getSprite();

            sprite.setPosition(sf::Vector2f(pos.x,pos.y));
            sprite.setScale(sf::Vector2f(scale.x * 4, scale.y * 4));
            window.draw(sprite);
        }

        for (auto e : m_entityManager.getEntities("Player"))
        {
            Vec2 pos = e->getComponent<CTransform>().pos - m_cameraPosition;
            Vec2 scale = e->getComponent<CTransform>().scale;
            sf::Sprite & sprite = e->getComponent<CAnimation>().animation.getSprite();

            sprite.setPosition(sf::Vector2f(pos.x,pos.y));
            sprite.setScale(sf::Vector2f(scale.x * 4, scale.y * 4));
            window.draw(sprite);
        }
    }

    // Draw Bounding Boxes
    if (m_drawCollision)
    {
        for (auto e : m_entityManager.getEntities())
        {
            Vec2 pos = e->getComponent<CTransform>().pos - m_cameraPosition;
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
        const int MAP_WIDTH_BLOCKS = 400;

        // Draw grid vertical lines
        const int verticalLines = ceil((float) widthWindow / widthCell);
        for (int i = 0; i < MAP_WIDTH_BLOCKS; i++) 
        {
            int x = widthCell * (i + 1) - m_cameraPosition.x;
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
        for (int gx = 0; gx < MAP_WIDTH_BLOCKS; gx++)
        {
            for (int gy = 0; gy < horizontalLines; gy++)
            {
                std::ostringstream oss;
                oss << "(" << gx << "," << gy << ")";
                m_gridText.setString(oss.str());

                // relative to top left of window
                int x = widthCell * gx - m_cameraPosition.x;
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