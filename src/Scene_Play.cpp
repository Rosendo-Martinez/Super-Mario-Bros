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

    sState(); // Here?
    sAnimation();
    sMovement();
    sCollision();
    sRender();
}

void Scene_Play::sAnimation()
{
    m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("MarioStand"); // temporary

    for (auto e: m_entityManager.getEntities())
    {
        e->getComponent<CAnimation>().animation.update();
    }
}

void Scene_Play::sState()
{
    CState& cState = m_player->getComponent<CState>();
    const CInput& cInput = m_player->getComponent<CInput>();
    const CTransform& cTransform = m_player->getComponent<CTransform>();

    const bool isAirborne = !cState.isGrounded;
    const bool isPressingLeft  = cInput.left;
    const bool isPressingRight = cInput.right;
    const bool isStandingStill = cTransform.velocity.x == 0;
    const bool isMovingRight   = cTransform.velocity.x > 0;
    const bool isMovingLeft    = cTransform.velocity.x < 0;

    // Decelerating: speed is decreasing (going to zero)
    // Accelerating: speed is increasing (going away from zero)
    bool isDeceleratingLeft  = false; 
    bool isDeceleratingRight = false; 
    bool isAcceleratingLeft  = false;
    bool isAcceleratingRight = false;

    if (isAirborne)
    {
        const bool isChangingMovementDirection    = (isMovingLeft && (isPressingRight && !isPressingLeft)) || (isMovingRight && (isPressingLeft && !isPressingRight)); // turning only, doesn't include stopping
        const bool isAboveInitialSpeedThresholdForVel = (cState.initialJumpXSpeed <= -m_airborneHK.INITIAL_SPEED_THRESHOLD_FOR_VEL || cState.initialJumpXSpeed >= m_airborneHK.INITIAL_SPEED_THRESHOLD_FOR_VEL);
        const double maxXSpeed                    = isAboveInitialSpeedThresholdForVel ? m_airborneHK.ABOVE_IST_SPEED_LIMIT_VEL : m_airborneHK.BELLOW_ISP_SPEED_LIMIT_VEL;
        const bool isBellowMaxSpeed               = (cTransform.velocity.x < maxXSpeed) && (cTransform.velocity.x > -maxXSpeed);

        isDeceleratingLeft              = (isMovingRight && isChangingMovementDirection);
        isDeceleratingRight             = (isMovingLeft && isChangingMovementDirection);
        isAcceleratingLeft              = (isPressingLeft && !isPressingRight) && (isMovingLeft || isStandingStill) && isBellowMaxSpeed;
        isAcceleratingRight             = isPressingRight && !isPressingLeft && (isStandingStill || isMovingRight) && isBellowMaxSpeed;
    }
    else // grounded
    {
        const bool isRunning                      = cInput.B;
        const bool isAtMaxWalkSpeed               = cTransform.velocity.x == m_groundedHK.MAX_WALK_SPEED || cTransform.velocity.x == -m_groundedHK.MAX_WALK_SPEED;
        const bool isAtMaxRunSpeed                = cTransform.velocity.x == m_groundedHK.MAX_RUN_SPEED || cTransform.velocity.x == -m_groundedHK.MAX_RUN_SPEED;
        const bool isWalkingButPastMaxWalkSpeed   = (cTransform.velocity.x > m_groundedHK.MAX_WALK_SPEED || cTransform.velocity.x < -m_groundedHK.MAX_WALK_SPEED) && !isRunning;
        const bool isChangingMovementDirection    = (isMovingLeft && (isPressingRight || !isPressingLeft)) || (isMovingRight && (isPressingLeft || !isPressingRight)); // stopping, or turning

        isDeceleratingLeft              = (isMovingRight && (isChangingMovementDirection || isWalkingButPastMaxWalkSpeed));
        isDeceleratingRight             = (isMovingLeft && (isChangingMovementDirection || isWalkingButPastMaxWalkSpeed));
        isAcceleratingLeft              = (isPressingLeft && !isPressingRight) && (isMovingLeft || isStandingStill) && (!isAtMaxWalkSpeed || isRunning) && !isAtMaxRunSpeed && !isWalkingButPastMaxWalkSpeed;
        isAcceleratingRight             = isPressingRight && !isPressingLeft && (isStandingStill || isMovingRight) && (!isAtMaxWalkSpeed || isRunning) && !isAtMaxRunSpeed && !isWalkingButPastMaxWalkSpeed;
    }

    if (isDeceleratingLeft)
    {
        cState.acceleration = Acceleration::DECELERATING_LEFT;
    }
    else if (isDeceleratingRight)
    {
        cState.acceleration = Acceleration::DECELERATING_RIGHT;
    }
    else if (isAcceleratingLeft)
    {
        cState.acceleration = Acceleration::ACCELERATING_LEFT;
    }
    else if (isAcceleratingRight)
    {
        cState.acceleration = Acceleration::ACCELERATING_RIGHT;
    }
    else
    {
        cState.acceleration = Acceleration::ZERO;
    }
}

void Scene_Play::sAirBorneMovement()
{
    CTransform& cTransform  = m_player->getComponent<CTransform>();
    const CInput& cInput          = m_player->getComponent<CInput>();
    const CState& cState          = m_player->getComponent<CState>();

    const bool isMovingRight                  = cTransform.velocity.x > 0;
    const bool isMovingLeft                   = cTransform.velocity.x < 0;
    const bool isAboveInitialSpeedThresholdForVel = (cState.initialJumpXSpeed <= -m_airborneHK.INITIAL_SPEED_THRESHOLD_FOR_VEL || cState.initialJumpXSpeed >= m_airborneHK.INITIAL_SPEED_THRESHOLD_FOR_VEL);

    // Decelerating: speed is decreasing (going to zero)
    // Accelerating: speed is increasing (going away from zero)
    const bool isDeceleratingLeft  = (cState.acceleration == Acceleration::DECELERATING_LEFT);
    const bool isDeceleratingRight = (cState.acceleration == Acceleration::DECELERATING_RIGHT);
    const bool isAcceleratingLeft  = (cState.acceleration == Acceleration::ACCELERATING_LEFT);
    const bool isAcceleratingRight = (cState.acceleration == Acceleration::ACCELERATING_RIGHT);

    const bool isAboveCurrentSpeedThresholdForAcc = (cTransform.velocity.x <= -m_airborneHK.CURRENT_SPEED_THRESHOLD_FOR_ACC || cTransform.velocity.x >= m_airborneHK.CURRENT_SPEED_THRESHOLD_FOR_ACC);
    const bool isAboveInitialSpeedThresholdForAcc = (cState.initialJumpXSpeed <= -m_airborneHK.INITIAL_SPEED_THRESHOLD_FOR_ACC || cState.initialJumpXSpeed >= m_airborneHK.INITIAL_SPEED_THRESHOLD_FOR_ACC);

    // Step 1: Figure out X acceleration for current frame
    double accelerationX = 0;
    if (isAboveCurrentSpeedThresholdForAcc)
    {
        accelerationX = m_airborneHK.ABOVE_CST_ACC;
    }
    else
    {
        accelerationX = m_airborneHK.BELOW_CST_ACC;
    }
    double decelerationX = 0;
    if (isAboveCurrentSpeedThresholdForAcc)
    {
        decelerationX = m_airborneHK.ABOVE_CST_DEC;
    }
    else if (isAboveInitialSpeedThresholdForAcc)
    {
        decelerationX = m_airborneHK.ABOVE_IST_DEC;
    }
    else
    {
        decelerationX = m_airborneHK.BELOW_IST_DEC;
    }

    if (isAcceleratingLeft)
    {
        cTransform.acc_x = -accelerationX;
    }
    else if (isAcceleratingRight)
    {
        cTransform.acc_x = accelerationX;
    }
    else if (isDeceleratingLeft)
    {
        cTransform.acc_x = -decelerationX;
    }
    else if (isDeceleratingRight)
    {
        cTransform.acc_x = decelerationX;
    }
    else
    {
        cTransform.acc_x = 0;
    }

    // Step 2: Use X acceleration to calculate X velocity
    cTransform.velocity.x += cTransform.acc_x;

    // Step 3: Apply speed limits or exception for X velocity
    if (isAboveInitialSpeedThresholdForVel)
    {
        if (cTransform.velocity.x > m_airborneHK.ABOVE_IST_SPEED_LIMIT_VEL)
        {
            cTransform.velocity.x = m_airborneHK.ABOVE_IST_SPEED_LIMIT_VEL;
        }
        else if (cTransform.velocity.x < -m_airborneHK.ABOVE_IST_SPEED_LIMIT_VEL)
        {
            cTransform.velocity.x = -m_airborneHK.ABOVE_IST_SPEED_LIMIT_VEL;
        }
    }
    else
    {
        if (cTransform.velocity.x > m_airborneHK.BELLOW_ISP_SPEED_LIMIT_VEL)
        {
            cTransform.velocity.x = m_airborneHK.BELLOW_ISP_SPEED_LIMIT_VEL;
        }
        else if (cTransform.velocity.x < -m_airborneHK.BELLOW_ISP_SPEED_LIMIT_VEL)
        {
            cTransform.velocity.x = -m_airborneHK.BELLOW_ISP_SPEED_LIMIT_VEL;
        }
    }
    
    const bool isPressingJump                         = cInput.A;
    const bool hadReducedGravity                      = (cTransform.acc_y == m_jumpVK.REDUCED_GRAVITY_S) || (cTransform.acc_y == m_jumpVK.REDUCED_GRAVITY_M);
    const bool isFalling                              = (cTransform.velocity.y >= 0);
    const bool isJustStartingNormalGravityPhaseOfJump = hadReducedGravity && (!isPressingJump || isFalling);

    // Step 4: Figure out Y acceleration
    if (isJustStartingNormalGravityPhaseOfJump)
    {
        if (cTransform.acc_y == m_jumpVK.REDUCED_GRAVITY_S)
        {
            cTransform.acc_y = m_jumpVK.GRAVITY_S;
        }
        else if (cTransform.acc_y == m_jumpVK.REDUCED_GRAVITY_M)
        {
            cTransform.acc_y = m_jumpVK.GRAVITY_M;
        }
        else
        {
            cTransform.acc_y = m_jumpVK.GRAVITY_L;
        }
    }

    // Step 5: Use Y acceleration to calculate Y velocity
    cTransform.velocity.y += cTransform.acc_y;

    // Step 6: Apply speed limits or exception for Y velocity
    if (cTransform.velocity.y > m_jumpVK.MAX_Y_SPEED)
    {
        cTransform.velocity.y = m_jumpVK.RESET_SPEED;
    }
    else if (cTransform.velocity.y < -m_jumpVK.MAX_Y_SPEED)
    {
        cTransform.velocity.y = -m_jumpVK.RESET_SPEED;
    }

    // Step 7: Use velocity to calculate player position
    cTransform.prevPos =  cTransform.pos;
    cTransform.pos += cTransform.velocity;
}

void Scene_Play::sGroundedMovement()
{
    CInput& cInput         = m_player->getComponent<CInput>();
    CTransform& cTransform = m_player->getComponent<CTransform>();
    CState& cState         = m_player->getComponent<CState>();

    const bool isPressingLeft            = cInput.left;
    const bool isPressingRight           = cInput.right;
    const bool isRunning                 = cInput.B;
    const bool isWalking                 = !cInput.B;
    const bool isMovingRight             = cTransform.velocity.x > 0;
    const bool isMovingLeft              = cTransform.velocity.x < 0;
    const bool isSkiddingInPreviousFrame = (cTransform.acc_x == m_groundedHK.SKID_DEC || cTransform.acc_x == -m_groundedHK.SKID_DEC);

    // Decelerating: speed is decreasing (going to zero)
    // Accelerating: speed is increasing (going away from zero)
    const bool isDeceleratingLeft  = (cState.acceleration == Acceleration::DECELERATING_LEFT);
    const bool isDeceleratingRight = (cState.acceleration == Acceleration::DECELERATING_RIGHT);
    const bool isAcceleratingLeft  = (cState.acceleration == Acceleration::ACCELERATING_LEFT);
    const bool isAcceleratingRight = (cState.acceleration == Acceleration::ACCELERATING_RIGHT);
    const bool isSkidding          = ((isMovingRight && isPressingLeft && !isPressingRight) || (isMovingLeft && isPressingRight && !isPressingLeft)) || ((isDeceleratingLeft || isDeceleratingRight) && isSkiddingInPreviousFrame);

    // Step 1: Figure out X acceleration for current frame
    double accelerationX = 0;
    if (isRunning)
    {
        accelerationX = m_groundedHK.RUN_ACC;
    }
    else
    {
        accelerationX = m_groundedHK.WALK_ACC;
    }
    double decelerationX = 0;
    if (isSkidding) 
    {
        decelerationX = m_groundedHK.SKID_DEC;
    }
    else
    {
        decelerationX = m_groundedHK.RELEASE_DEC;
    }

    if (isDeceleratingRight)
    {
        cTransform.acc_x = decelerationX;
    }
    else if (isDeceleratingLeft)
    {
        cTransform.acc_x = -decelerationX;
    }
    else if (isAcceleratingRight)
    {
        cTransform.acc_x = accelerationX;
    }
    else if (isAcceleratingLeft)
    {
        cTransform.acc_x = -accelerationX;
    }
    else
    {
        cTransform.acc_x = 0;
    }

    // Step 2: Use X acceleration to calculate X velocity
    cTransform.velocity.x += cTransform.acc_x;

    const bool isPastMaxWalkSpeed      = cTransform.velocity.x > m_groundedHK.MAX_WALK_SPEED || cTransform.velocity.x < -m_groundedHK.MAX_WALK_SPEED;
    const bool isPastMaxRunSpeed       = cTransform.velocity.x > m_groundedHK.MAX_RUN_SPEED || cTransform.velocity.x < -m_groundedHK.MAX_RUN_SPEED;
    const bool isBellowMinWalkSpeed    = cTransform.velocity.x < m_groundedHK.MIN_WALK_SPEED && cTransform.velocity.x > -m_groundedHK.MIN_WALK_SPEED;
    const bool isBellowTurnAroundSpeed = cTransform.velocity.x < m_groundedHK.SKID_TURNAROUND_SPEED && cTransform.velocity.x > -m_groundedHK.SKID_TURNAROUND_SPEED;

    // Step 3: Apply speed limits or exception for X velocity
    // Mario is walking right and past max walk speed
    if (isPastMaxWalkSpeed && isAcceleratingRight && isWalking)
    {
        cTransform.velocity.x = m_groundedHK.MAX_WALK_SPEED;
    }
    // Mario is walking left and past max walk speed
    else if (isPastMaxWalkSpeed && isAcceleratingLeft && isWalking)
    {
        cTransform.velocity.x = -m_groundedHK.MAX_WALK_SPEED;
    }
    // Mario is running right and past max run speed
    else if (isPastMaxRunSpeed && isAcceleratingRight && isRunning)
    {
        cTransform.velocity.x = m_groundedHK.MAX_RUN_SPEED;   
    }
    // Mario is running left and past max run speed
    else if (isPastMaxRunSpeed && isAcceleratingLeft && isRunning)
    {
        cTransform.velocity.x = -m_groundedHK.MAX_RUN_SPEED;   
    }
    // Mario is accelerating right and is bellow min x speed
    else if (isBellowMinWalkSpeed && isAcceleratingRight)
    {
        cTransform.velocity.x = m_groundedHK.MIN_WALK_SPEED;
    }
    // Mario is accelerating left and is bellow min x speed
    else if (isBellowMinWalkSpeed && isAcceleratingLeft)
    {
        cTransform.velocity.x = -m_groundedHK.MIN_WALK_SPEED;
    }
    // Mario is decelerating and is bellow min x speed
    else if ((isBellowMinWalkSpeed || (isBellowTurnAroundSpeed && isSkidding)) && (isDeceleratingLeft || isDeceleratingRight))
    {
        cTransform.velocity.x = 0;
    }

    const double xSpeed                  = cTransform.velocity.x;
    const bool canJump                   = cInput.canJump;
    const bool isPressingJump            = cInput.A;
    const bool isAtSmallHorizontalSpeed  = (m_jumpVK.SMALL_SPEED_THRESHOLD > xSpeed) && (-m_jumpVK.SMALL_SPEED_THRESHOLD < xSpeed);
    const bool isAtMediumHorizontalSpeed = (m_jumpVK.MEDIUM_SPEED_THRESHOLD >= xSpeed) && (-m_jumpVK.MEDIUM_SPEED_THRESHOLD <= xSpeed) && (!isAtSmallHorizontalSpeed);
    const bool isJustStartingJump        = canJump && isPressingJump;

    // Step 4: Check if player is about to jump
    if (isJustStartingJump)
    {
        if (isAtSmallHorizontalSpeed)
        {
            cTransform.acc_y = m_jumpVK.REDUCED_GRAVITY_S;
        }
        else if (isAtMediumHorizontalSpeed)
        {
            cTransform.acc_y = m_jumpVK.REDUCED_GRAVITY_M;
        }
        else
        {
            cTransform.acc_y = m_jumpVK.REDUCED_GRAVITY_L;
        }
    }

    // Step 5: Use Y acceleration to calculate Y velocity
    cTransform.velocity.y += cTransform.acc_y;

    // Step 6: Apply speed limits or exception for Y velocity
    if (isJustStartingJump)
    {
        if (isAtSmallHorizontalSpeed)
        {
            cTransform.velocity.y = -m_jumpVK.INITIAL_VELOCITY_S;
        }
        else if (isAtMediumHorizontalSpeed)
        {
            cTransform.velocity.y = -m_jumpVK.INITIAL_VELOCITY_M;
        }
        else
        {
            cTransform.velocity.y = -m_jumpVK.INITIAL_VELOCITY_L;
        }

        cInput.canJump = false;
        cState.initialJumpXSpeed = cTransform.velocity.x;
        cState.isGrounded = false;
    }

    // Step 7: Use velocity to calculate player position
    cTransform.prevPos =  cTransform.pos;
    cTransform.pos += cTransform.velocity;
}

void Scene_Play::sMovement()
{
    CInput& cInput = m_player->getComponent<CInput>();
    CTransform& cTransform = m_player->getComponent<CTransform>();
    CState& cState = m_player->getComponent<CState>();
    bool isAirborne = !cState.isGrounded;

    if (isAirborne)
    {
        sAirBorneMovement();
        // std::cout << "Airborne\n";
    }
    else
    {
        sGroundedMovement();
        // std::cout << "Grounded\n";
    }

    // std::cout << "A = (" << cTransform.acc_x << ", " << cTransform.acc_y << ")\n";
    // std::cout << "V = (" << cTransform.velocity.x << ", " << cTransform.velocity.y << ")\n";

    // Player fell off the map
    if (cTransform.pos.y - 64/2 > m_game->window().getSize().y)
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
            m_player->getComponent<CState>().isGrounded = true;
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
        m_player->getComponent<CState>().isGrounded
    )
    {
        m_player->getComponent<CState>().isGrounded = false;
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