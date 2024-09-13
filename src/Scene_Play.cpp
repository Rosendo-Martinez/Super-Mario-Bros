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
    registerAction(sf::Keyboard::B, "RUN");
    registerAction(sf::Keyboard::V, "JUMP");
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

    m_player->getComponent<CTransform>().acc_y = 0.375 * 4;
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
    else if ((m_player->getComponent<CState>().state == "Jumping" || m_player->getComponent<CState>().state == "Falling") && m_player->getComponent<CAnimation>().animation.getName() != "MarioAir")
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
    double v_min = 0.07421875 * 4;
    double v_max_walk = 1.5625 * 4;
    double v_max_run = 2.5625 * 4;
    double v_turnaround = 0.5625 * 4;
    double a_walk = 0.037109375 * 4;
    double a_run = 0.0556640625 * 4;
    double d_release = 0.05078125 * 4;
    double d_skid = 0.1015625 * 4;

    bool isPressingLeft                 = m_player->getComponent<CInput>().left;
    bool isPressingRight                = m_player->getComponent<CInput>().right;
    bool isRunning                      = m_player->getComponent<CInput>().B;
    bool isPressingBothOrNeither        = (!isPressingLeft && !isPressingRight) || (isPressingLeft && isPressingRight); // pressing both left and right or pressing neither
    bool isStandingStill                = m_player->getComponent<CTransform>().velocity.x == 0;
    bool isMovingRight                  = m_player->getComponent<CTransform>().velocity.x > 0;
    bool isMovingLeft                   = m_player->getComponent<CTransform>().velocity.x < 0;
    bool isAtMaxWalkSpeed               = m_player->getComponent<CTransform>().velocity.x == v_max_walk || m_player->getComponent<CTransform>().velocity.x == -v_max_walk;
    bool isAtMaxRunSpeed                = m_player->getComponent<CTransform>().velocity.x == v_max_run || m_player->getComponent<CTransform>().velocity.x == -v_max_run;
    bool isWalkingButPastMaxWalkSpeed   = (m_player->getComponent<CTransform>().velocity.x > v_max_walk || m_player->getComponent<CTransform>().velocity.x < -v_max_walk) && !isRunning;
    bool isSkiddingInPreviousFrame      = (m_player->getComponent<CTransform>().acc_x == d_skid || m_player->getComponent<CTransform>().acc_x == -d_skid);

    // Decelerating: moving to a speed of zero
    // Accelerating: moving to a speed (+ or -) away from zero
    bool isDeceleratingLeft = (isMovingRight && ((isPressingBothOrNeither || isPressingLeft) || isWalkingButPastMaxWalkSpeed));
    bool isDeceleratingRight = (isMovingLeft && ((isPressingBothOrNeither || isPressingRight) || isWalkingButPastMaxWalkSpeed));
    bool isAcceleratingLeft = isPressingLeft && !isPressingRight && (isMovingLeft || isStandingStill) && (!isAtMaxWalkSpeed || isRunning) && !isAtMaxRunSpeed && !isWalkingButPastMaxWalkSpeed;
    bool isAcceleratingRight = isPressingRight && !isPressingLeft && (isStandingStill || isMovingRight) && (!isAtMaxWalkSpeed || isRunning) && !isAtMaxRunSpeed && !isWalkingButPastMaxWalkSpeed;
    bool isNotAcceleratingOrDecelerating = !isDeceleratingLeft && !isDeceleratingRight && !isAcceleratingLeft && !isAcceleratingRight;
    bool isSkidding = ((isMovingRight && isPressingLeft && !isPressingRight) || (isMovingLeft && isPressingRight && !isPressingLeft)) || ((isDeceleratingLeft || isDeceleratingRight) && isSkiddingInPreviousFrame);
    bool isAirborne = m_player->getComponent<CState>().state == "Jumping" || m_player->getComponent<CState>().state == "Falling"; 
    
    // Step 1: Figure out X acceleration for current frame
    if (!isAirborne)
    {
        // std::cout << "Grounded Acc, ";
        if (isDeceleratingRight)
        {
            if (isSkidding) 
            {
                m_player->getComponent<CTransform>().acc_x = d_skid;
            }
            else
            {
                m_player->getComponent<CTransform>().acc_x = d_release;
            }
        }
        else if (isDeceleratingLeft)
        {
            if (isSkidding || m_player->getComponent<CTransform>().acc_x == -d_skid) 
            {
                m_player->getComponent<CTransform>().acc_x = -d_skid;
            }
            else
            {
                m_player->getComponent<CTransform>().acc_x = -d_release;
            }
        }
        else if (isNotAcceleratingOrDecelerating)
        {
            m_player->getComponent<CTransform>().acc_x = 0;
        }
        else if (isAcceleratingRight)
        {
            if (isRunning)
            {
                m_player->getComponent<CTransform>().acc_x = a_run;
            }
            else 
            {
                m_player->getComponent<CTransform>().acc_x = a_walk;
            }
        }
        else if (isAcceleratingLeft)
        {
            if (isRunning)
            {
                m_player->getComponent<CTransform>().acc_x = -a_run;
            }
            else
            {
                m_player->getComponent<CTransform>().acc_x = -a_walk;
            }
        }
    }
    // Mario is airborne
    else
    {
        // std::cout << "Airborne Acc, ";
        double currentSpeed = 1.5625 * 4;
        double startingJumpSpeed = 1.8125 * 4;
        double smallAcceleration = 0.037109375 * 4; // 0x00098
        double largeAcceleration = 0.0556640625 * 4; // 0x000E4
        double largeDeceleration = 0.0556640625 * 4; // 0x000E4
        double mediumDeceleration = 0.05078125 * 4; // 0x000D0
        double smallDeceleration = 0.037109375 * 4; // 0x00098

        if (isAcceleratingLeft)
        {
            if (m_player->getComponent<CTransform>().velocity.x <= -currentSpeed)
            {
                m_player->getComponent<CTransform>().acc_x = -largeAcceleration;
            }
            else
            {
                m_player->getComponent<CTransform>().acc_x = -smallAcceleration;
            }
        }
        else if (isAcceleratingRight)
        {
            if (m_player->getComponent<CTransform>().velocity.x >= currentSpeed)
            {
                m_player->getComponent<CTransform>().acc_x = largeAcceleration;
            }
            else
            {
                m_player->getComponent<CTransform>().acc_x = smallAcceleration;
            }
        }
        else if (isDeceleratingLeft)
        {
            if (m_player->getComponent<CTransform>().velocity.x >= currentSpeed)
            {
                m_player->getComponent<CTransform>().acc_x = -largeDeceleration;
            }
            else if (m_player->getComponent<CState>().initialJumpXSpeed >= startingJumpSpeed)
            {
                m_player->getComponent<CTransform>().acc_x = -mediumDeceleration;
            }
            else
            {
                m_player->getComponent<CTransform>().acc_x = -smallDeceleration;
            }
        }
        else if (isDeceleratingRight)
        {
            if (m_player->getComponent<CTransform>().velocity.x <= -currentSpeed)
            {
                m_player->getComponent<CTransform>().acc_x = largeDeceleration;
            }
            else if (m_player->getComponent<CState>().initialJumpXSpeed <= -startingJumpSpeed)
            {
                m_player->getComponent<CTransform>().acc_x = mediumDeceleration;
            }
            else
            {
                m_player->getComponent<CTransform>().acc_x = smallDeceleration;
            }
        }
        // Player is not accelerating
        else
        {
            m_player->getComponent<CTransform>().acc_x = 0;
        }
    }


    // Step 2: Use X acceleration to calculate X velocity
    m_player->getComponent<CTransform>().velocity.x += m_player->getComponent<CTransform>().acc_x;

    // Step 3: Apply speed limits or exception for X velocity
    if (!isAirborne)
    {
        // std::cout << "Grounded Vel.\n";
        bool isPastMaxWalkSpeed   = m_player->getComponent<CTransform>().velocity.x > v_max_walk || m_player->getComponent<CTransform>().velocity.x < -v_max_walk;
        bool isPastMaxRunSpeed = m_player->getComponent<CTransform>().velocity.x > v_max_run || m_player->getComponent<CTransform>().velocity.x < -v_max_run;
        bool isBellowMinWalkSpeed = m_player->getComponent<CTransform>().velocity.x < v_min && m_player->getComponent<CTransform>().velocity.x > -v_min;
        bool isBellowTurnAroundSpeed = m_player->getComponent<CTransform>().velocity.x < v_turnaround && m_player->getComponent<CTransform>().velocity.x > -v_turnaround;

        // Apply speed limits if needed
        if (isPastMaxWalkSpeed && isAcceleratingRight && !isRunning)
        {
            m_player->getComponent<CTransform>().velocity.x = v_max_walk;
        }
        else if (isPastMaxWalkSpeed && isAcceleratingLeft && !isRunning)
        {
            m_player->getComponent<CTransform>().velocity.x = -v_max_walk;
        }
        else if (isPastMaxRunSpeed && isAcceleratingRight && isRunning)
        {
            m_player->getComponent<CTransform>().velocity.x = v_max_run;   
        }
        else if (isPastMaxRunSpeed && isAcceleratingLeft && isRunning)
        {
            m_player->getComponent<CTransform>().velocity.x = -v_max_run;   
        }
        else if (isBellowMinWalkSpeed && isAcceleratingRight)
        {
            m_player->getComponent<CTransform>().velocity.x = v_min;
        }
        else if (isBellowMinWalkSpeed && isAcceleratingLeft)
        {
            m_player->getComponent<CTransform>().velocity.x = -v_min;
        }
        else if ((isBellowMinWalkSpeed || (isBellowTurnAroundSpeed && isSkidding)) && (isDeceleratingLeft || isDeceleratingRight))
        {
            m_player->getComponent<CTransform>().velocity.x = 0;
        }
    }
    // Mario is airborne
    else
    {
        // std::cout << "Airborne Vel.\n";
        double startingJumpSpeed = 1.8125 * 4;
        double smallMomentumLimit = 1.5625 * 4;
        double largeMomentumLimit = 2.5625 * 4;

        if (m_player->getComponent<CState>().initialJumpXSpeed < startingJumpSpeed && m_player->getComponent<CTransform>().velocity.x > smallMomentumLimit)
        {
            m_player->getComponent<CTransform>().velocity.x = smallMomentumLimit;
        } 
        else if (m_player->getComponent<CTransform>().velocity.x > largeMomentumLimit)
        {
             m_player->getComponent<CTransform>().velocity.x = largeMomentumLimit;
        }
    }

    // std::cout << "Vel.x: " << m_player->getComponent<CTransform>().velocity.x << "\n";
    // std::cout << "Acc.c: " << m_player->getComponent<CTransform>().acc_x << "\n\n";

    double smallHorizontalSpeeds[] {0, 1 * 4}; // [Included, Excluded]
    double mediumHorizontalSpeeds[] {1 * 4, 2.312255859375 * 4}; // [Included, Included]
    double reducedGravityForSHS = 0.125 * 4; // 0.5
    double reducedGravityForMHS = 0.1171875 * 4; // 0.468
    double gravityForSHS = 0.4375 * 4; // 1.75
    double gravityForMHS = 0.375 * 4; // 1.5
    double initialJumpSpeedForSHS = 4 * 4; // 16
    double initialJumpSpeedForMHS = 4 * 4; // 16
    double maxVerticalSpeed = 4.5 * 4; // 18
    double resetVerticalSpeed = 4 * 4; // 16

    double xSpeed = m_player->getComponent<CTransform>().velocity.x;
    bool canJump = m_player->getComponent<CInput>().canJump;
    bool isPressingJump = m_player->getComponent<CInput>().A;
    bool isAtSmallHorizontalSpeed = (smallHorizontalSpeeds[0] <= xSpeed && smallHorizontalSpeeds[1] > xSpeed) || (-smallHorizontalSpeeds[0] >= xSpeed && -smallHorizontalSpeeds[1] < xSpeed);
    bool isAtMediumHorizontalSpeed = (mediumHorizontalSpeeds[0] <= xSpeed && mediumHorizontalSpeeds[1] >= xSpeed) || (-mediumHorizontalSpeeds[0] >= xSpeed && -mediumHorizontalSpeeds[1] <= xSpeed);
    bool hasReducedGravity = (m_player->getComponent<CTransform>().acc_y == reducedGravityForSHS || m_player->getComponent<CTransform>().acc_y == reducedGravityForMHS);
    bool isFalling = m_player->getComponent<CTransform>().velocity.y >= 0;

    // Step 4: Figure out Y acceleration
    if (canJump && isPressingJump)
    {
        if (isAtSmallHorizontalSpeed)
        {
            m_player->getComponent<CTransform>().acc_y = reducedGravityForSHS;
        }
        else if (isAtMediumHorizontalSpeed || true)
        {
            m_player->getComponent<CTransform>().acc_y = reducedGravityForMHS;
        }
    }
    else if (hasReducedGravity && (!isPressingJump || isFalling))
    {
        if (m_player->getComponent<CTransform>().acc_y == reducedGravityForSHS)
        {
            m_player->getComponent<CTransform>().acc_y = gravityForSHS;
        }
        else if (m_player->getComponent<CTransform>().acc_y == reducedGravityForMHS || true)
        {
            m_player->getComponent<CTransform>().acc_y = gravityForMHS;
        }
    }

    m_player->getComponent<CTransform>().velocity.y += m_player->getComponent<CTransform>().acc_y;

    // Step 5: Apply speed limits or exception for Y velocity
    if (canJump && isPressingJump)
    {
        if (isAtSmallHorizontalSpeed)
        {
            m_player->getComponent<CTransform>().velocity.y = -initialJumpSpeedForSHS;
        }
        else if (isAtMediumHorizontalSpeed || true)
        {
            m_player->getComponent<CTransform>().velocity.y = -initialJumpSpeedForMHS;
        }

        m_player->getComponent<CInput>().canJump = false;
        m_player->getComponent<CState>().initialJumpXSpeed = m_player->getComponent<CTransform>().velocity.x;
        m_player->getComponent<CState>().state = "Jumping";
    }
    if (m_player->getComponent<CTransform>().velocity.y > maxVerticalSpeed)
    {
        m_player->getComponent<CTransform>().velocity.y = resetVerticalSpeed;
    }

    // std::cout << "A_y: " << m_player->getComponent<CTransform>().acc_y << "\n";
    // std::cout << "V_y: " << m_player->getComponent<CTransform>().velocity.y << "\n";

    // Step 6: Use velocity to calculate player position
    m_player->getComponent<CTransform>().prevPos =  m_player->getComponent<CTransform>().pos;
    m_player->getComponent<CTransform>().pos += m_player->getComponent<CTransform>().velocity;

    // Player fell off the map
    if (m_player->getComponent<CTransform>().pos.y - 64/2 > m_game->window().getSize().y)
    {
        m_player->destroy();
    }
}

void Scene_Play::sEnemySpawn()
{
}

void Scene_Play::sCollision()
{
    // Basic idea behind player-block collisions is that the player only needs to resolve collisions for at most one block in each collision direction.
    // For example, if the player collides with 2 blocks and both collisions are from the left, then the player really only needs to do
    // collisions resolution for one since fixing one should also fix the other.
    // Mario can collide with at most 2 blocks in collisions where mario came from the left, right, top, or bottom relative to the block.
    // However, mario can only collide with 1 block diagonally at at time.

    std::shared_ptr<Entity> bottomHitBlock = nullptr;
    std::shared_ptr<Entity> leftHitBlock = nullptr;
    std::shared_ptr<Entity> rightHitBlock = nullptr;
    std::shared_ptr<Entity> topHitBlock = nullptr;
    std::shared_ptr<Entity> topLeftCornerHitBlock = nullptr;
    std::shared_ptr<Entity> topRightCornerHitBlock = nullptr;
    std::shared_ptr<Entity> bottomLeftCornerHitBlock = nullptr;
    std::shared_ptr<Entity> bottomRightCornerHitBlock = nullptr;

    // COLLISION DETECTION for player-block collisions
    for (auto currentBlock : m_entityManager.getEntities("Tile"))
    {
        Vec2 overlap = Physics::GetOverlap(m_player, currentBlock);
        Vec2 prevOverlap = Physics::GetPreviousOverlap(m_player, currentBlock);
        // if player collides with block
        if (Physics::IsCollision(overlap))
        {
            // Collision direction is the direction which mario came from relative to block.
            CollisionDirection collisionDir = Physics::GetCollisionDirection(prevOverlap, m_player->getComponent<CTransform>().prevPos, currentBlock->getComponent<CTransform>().pos);

            // Mario hit the bottom of the block.
            if (collisionDir == CollisionDirection::BOTTOM)
            {
                if (bottomHitBlock == nullptr || overlap.x > Physics::GetOverlap(m_player, bottomHitBlock).x)
                {
                    bottomHitBlock = currentBlock;
                }
            }
            // Mario hit the left side of the block.
            else if (collisionDir == CollisionDirection::LEFT)
            {
                if (leftHitBlock == nullptr || currentBlock->getComponent<CTransform>().pos.y < leftHitBlock->getComponent<CTransform>().pos.y)
                {
                    leftHitBlock = currentBlock;
                }
            }
            // Mario hit the right side of the block.
            else if (collisionDir == CollisionDirection::RIGHT)
            {
                if (rightHitBlock == nullptr || currentBlock->getComponent<CTransform>().pos.y < rightHitBlock->getComponent<CTransform>().pos.y)
                {
                    rightHitBlock = currentBlock;
                }
            }
            // Mario hit the top of the block.
            else if (collisionDir == CollisionDirection::TOP)
            {
                if (topHitBlock == nullptr || overlap.x > Physics::GetOverlap(m_player, topHitBlock).x)
                {
                    topHitBlock = currentBlock;
                }
            }
            // Mario hit the top left corner of the block.
            else if (collisionDir == CollisionDirection::DIAGONAL_TOP_LEFT)
            {
                topLeftCornerHitBlock = currentBlock;
            }
            // Mario hit the top right corner of the block.
            else if (collisionDir == CollisionDirection::DIAGONAL_TOP_RIGHT)
            {
                topRightCornerHitBlock = currentBlock;
            }
            // Mario hit the bottom left corner of the block.
            else if (collisionDir == CollisionDirection::DIAGONAL_BOTTOM_LEFT)
            {
                bottomLeftCornerHitBlock = currentBlock;
            }
            // Mario hit the bottom right corner of the block.
            else if (collisionDir == CollisionDirection::DIAGONAL_BOTTOM_RIGHT)
            {
                bottomRightCornerHitBlock = currentBlock;
            }
            else
            {
                std::cout << "Error: unsupported collision direction: (int) " << (int) collisionDir << "\n";
            }
        }
    }

    // COLLISION RESOLUTION for player-block collisions
    if (bottomHitBlock != nullptr)
    {
        Vec2 overlap = Physics::GetOverlap(m_player, bottomHitBlock);

        m_player->getComponent<CTransform>().pos.y += overlap.y;
        m_player->getComponent<CTransform>().velocity.y = 0;

        // Special blocks collision
        if (bottomHitBlock->getComponent<CAnimation>().animation.getName() == "QuestionMarkBlink")
        {
            auto hitQuestionBlock = m_entityManager.addEntity("Tile");
            hitQuestionBlock->addComponent<CAnimation>(m_game->assets().getAnimation("QuestionMarkBlockHit"), true);
            hitQuestionBlock->addComponent<CTransform>(bottomHitBlock->getComponent<CTransform>().pos);
            hitQuestionBlock->addComponent<CBoundingBox>(Vec2(64, 64));

            bottomHitBlock->destroy();
        }
        else if (bottomHitBlock->getComponent<CAnimation>().animation.getName() == "Brick")
        {
            bottomHitBlock->destroy();
        }
    }
    if (leftHitBlock != nullptr) 
    {
        // TODO: Pull up mechanic for mario

        Vec2 overlap = Physics::GetOverlap(m_player, leftHitBlock);

        if (Physics::IsCollision(overlap))
        {
            m_player->getComponent<CTransform>().pos.x -= overlap.x;
        }
    }
    if (rightHitBlock != nullptr)
    {
        // TODO: Pull up mechanic for mario

        Vec2 overlap = Physics::GetOverlap(m_player, rightHitBlock);

        if (Physics::IsCollision(overlap))
        {
            m_player->getComponent<CTransform>().pos.x += overlap.x;
        }
    }
    if (topHitBlock != nullptr)
    {
        Vec2 overlap = Physics::GetOverlap(m_player, topHitBlock);

        if (Physics::IsCollision(overlap))
        {
            m_player->getComponent<CTransform>().pos.y -= overlap.y;
            m_player->getComponent<CTransform>().velocity.y = 0;
            m_player->getComponent<CInput>().canJump = true;

            if (m_player->getComponent<CInput>().left || m_player->getComponent<CInput>().right)
            {
                m_player->getComponent<CState>().state = "Running";
            }
            else
            {
                m_player->getComponent<CState>().state = "Standing";
            }
        }
    }
    if (topLeftCornerHitBlock != nullptr)
    {
        Vec2 overlap = Physics::GetOverlap(m_player, topLeftCornerHitBlock);

        if (Physics::IsCollision(overlap))
        {
            m_player->getComponent<CTransform>().pos.x -= overlap.x;
        }
    }
    if (topRightCornerHitBlock != nullptr)
    {
        Vec2 overlap = Physics::GetOverlap(m_player, topRightCornerHitBlock);

        if (Physics::IsCollision(overlap))
        {
            m_player->getComponent<CTransform>().pos.x += overlap.x;
        }
    }
    if (bottomLeftCornerHitBlock != nullptr)
    {
        Vec2 overlap = Physics::GetOverlap(m_player, bottomLeftCornerHitBlock);

        if (Physics::IsCollision(overlap))
        {
            m_player->getComponent<CTransform>().pos.x -= overlap.x;
        }
    }
    if (bottomRightCornerHitBlock != nullptr)
    {
        Vec2 overlap = Physics::GetOverlap(m_player, bottomRightCornerHitBlock);

        if (Physics::IsCollision(overlap))
        {
            m_player->getComponent<CTransform>().pos.x += overlap.x;
        }
    }

    // Mario is colliding with NO blocks.
    if 
    (
        bottomHitBlock == nullptr &&
        leftHitBlock == nullptr &&
        rightHitBlock == nullptr &&
        topHitBlock == nullptr &&
        topLeftCornerHitBlock == nullptr &&
        topRightCornerHitBlock == nullptr &&
        bottomLeftCornerHitBlock == nullptr &&
        bottomRightCornerHitBlock == nullptr &&
        (m_player->getComponent<CState>().state == "Running" ||
        m_player->getComponent<CState>().state == "Standing") 
    )
    {
        m_player->getComponent<CState>().state = "Falling";
        m_player->getComponent<CState>().initialJumpXSpeed = m_player->getComponent<CTransform>().velocity.x;
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
        for (auto e : m_entityManager.getEntities("Decoration"))
        {
            Vec2 pos = e->getComponent<CTransform>().pos - m_cameraPosition;
            Vec2 scale = e->getComponent<CTransform>().scale;
            sf::Sprite & sprite = e->getComponent<CAnimation>().animation.getSprite();

            sprite.setPosition(sf::Vector2f(pos.x,pos.y));
            sprite.setScale(sf::Vector2f(scale.x, scale.y));
            window.draw(sprite);
        }

        for (auto e : m_entityManager.getEntities("Tile"))
        {
            Vec2 pos = e->getComponent<CTransform>().pos - m_cameraPosition;
            Vec2 scale = e->getComponent<CTransform>().scale;
            sf::Sprite & sprite = e->getComponent<CAnimation>().animation.getSprite();

            sprite.setPosition(sf::Vector2f(pos.x,pos.y));
            sprite.setScale(sf::Vector2f(scale.x, scale.y));
            window.draw(sprite);
        }

        for (auto e : m_entityManager.getEntities("Player"))
        {
            Vec2 pos = e->getComponent<CTransform>().pos - m_cameraPosition;
            Vec2 scale = e->getComponent<CTransform>().scale;
            sf::Sprite & sprite = e->getComponent<CAnimation>().animation.getSprite();

            sprite.setPosition(sf::Vector2f(pos.x,pos.y));
            sprite.setScale(sf::Vector2f(scale.x, scale.y));
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

        if (action.name() == "RUN")
        {
            m_player->getComponent<CInput>().B = true;
        }

        if (action.name() == "JUMP")
        {
            m_player->getComponent<CInput>().A = true;
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

        if (action.name() == "RUN")
        {
            m_player->getComponent<CInput>().B = false;
        }

        if (action.name() == "JUMP")
        {
            m_player->getComponent<CInput>().A = false;
        }
    }
}

void Scene_Play::sDebug()
{   
}

void Scene_Play::onEnd() // before scene ends change to menu scene
{
}