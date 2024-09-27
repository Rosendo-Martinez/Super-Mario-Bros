#include "Scene_Play.h"
#include "Physics.h"
#include <iostream>
#include <sstream> 
#include <cmath>
#include <fstream>

/*
Initializes the object. 

Should be called once and only once before any other class methods are called.
*/
void Scene_Play::init()
{
    // Bind keyboard keys to actions
    registerAction(sf::Keyboard::G, "TOGGLE_GRID");
    registerAction(sf::Keyboard::C, "TOGGLE_BOUNDING_BOXES");
    registerAction(sf::Keyboard::T, "TOGGLE_TEXTURES");
    registerAction(sf::Keyboard::W, "UP");
    registerAction(sf::Keyboard::S, "DOWN");
    registerAction(sf::Keyboard::A, "LEFT");
    registerAction(sf::Keyboard::D, "RIGHT");
    registerAction(sf::Keyboard::B, "RUN");
    registerAction(sf::Keyboard::V, "JUMP");

    // Initialize debugging grid
    m_gridText.setFont(m_game->assets().getFont("Grid"));
    m_gridText.setCharacterSize(12);
    m_gridText.setFillColor(sf::Color::White);

    // Spawn player, and load the level
    spawnPlayer();
    loadLevel();
}

/**
 * Transforms the grid coordinate representation of the position of a entity
 * to the cartesian coordinate representation.
 * 
 * Note, unlike the grid coordinate representation of the entity, where the bottom left corner of the grid cell
 * lines up with the bottom left corner of the entity, in the cartesian coordinate representation the entity is
 * represented by a point at its center.
 */
Vec2 Scene_Play::gridToCartesianRepresentation(float gridX, float gridY, std::shared_ptr<Entity> entity)
{
    const sf::FloatRect size = entity->getComponent<CAnimation>().animation.getSprite().getGlobalBounds();

    return gridToCartesianRepresentation(Vec2(gridX,gridY), Vec2(size.width, size.height));
}

/**
 * Transforms the grid coordinate representation of the position of a entity
 * to the cartesian coordinate representation.
 * 
 * Note, unlike the grid coordinate representation of the entity, where the bottom left corner of the grid cell
 * lines up with the bottom left corner of the entity, in the cartesian coordinate representation the entity is
 * represented by a point at its center.
 */
Vec2 Scene_Play::gridToCartesianRepresentation(Vec2 gridPos, Vec2 size)
{
    const sf::RenderWindow & window = m_game->window();
    const int heighGrid = window.getSize().y;
    const int widthGrid = window.getSize().x;

    // Bottom left corner of entity in cartesian coordinates (from grid coordinates)
    const Vec2 bottomLeft(m_gridCellSize.x * (gridPos.x), heighGrid - m_gridCellSize.y * (gridPos.y));

    // Center of entity
    const Vec2 center(bottomLeft.x + size.x / 2, bottomLeft.y - size.y / 2);

    return center;
}

/**
 * Creates Tile, and Decoration type entities.
 */
void Scene_Play::createStaticEntity(std::string type, std::string animation, float gx, float gy)
{
    if (type != "Tile" && type != "Decoration")
    {
        std::cout << "Error: can only create Tile or Decoration type entities!\n";
        return;
    }

    // Create entity
    auto e = m_entityManager.addEntity(type);

    // Add components
    e->addComponent<CAnimation>(m_game->assets().getAnimation(animation), true);
    e->addComponent<CTransform>(gridToCartesianRepresentation(gx,gy,e));

    // Tiles have bounding boxes (i.e collisions)
    if (type == "Tile")
    {
        e->addComponent<CBoundingBox>(Vec2(64,64));
    }
}

/**
 * Creates Goomba type entities.
 */
void Scene_Play::createEnemyEntity(std::string type, float gx, float gy, float activationDistance)
{
    if (type != "Goomba")
    {
        std::cout << "Error: " << type << " creation is not yet supported!\n";
        return;
    }

    // Kinematic constantans
    const float GOOMBA_WALK = -m_groundedHK.MAX_WALK_SPEED;
    const float GOOMBA_GRAVITY = m_jumpVK.GRAVITY_L;

    auto e = m_entityManager.addEntity("Enemy");

    // Add components
    e->addComponent<CAnimation>(m_game->assets().getAnimation("GoombaWalk"), true);
    e->addComponent<CBoundingBox>(Vec2(64,64));
    CTransform& goombaCT = e->addComponent<CTransform>(gridToCartesianRepresentation(gx,gy,e));
    CEnemy& goombaCE =  e->addComponent<CEnemy>();

    // Initialize components
    goombaCT.velocity.x = GOOMBA_WALK;
    goombaCT.acc_y = GOOMBA_GRAVITY;
    goombaCE.activation_x = (gx - activationDistance) * 64;
}


/**
 * Loads the level.
 * 
 * If level was already loaded, then calling this function reloads the level.
 */
void Scene_Play::loadLevel()
{
    std::ifstream levelSpec (m_levelPath);

    if (!levelSpec.is_open())
    {
        std::cout << "Error: level specification file could not be open.\n";
        return;
    }

    while (levelSpec)
    {
        std::string type;
        levelSpec >> type;

        if (type == "Tile" || type == "Decoration")
        {
            std::string animationName;
            float gx;
            float gy;

            levelSpec >> animationName >> gx >> gy;

            createStaticEntity(type, animationName, gx, gy);
            continue;
        }
        else if (type == "TileRangeHorizontal" || type == "DecorationRangeHorizontal" || type == "TileRangeVertical" || type == "DecorationRangeVertical")
        {
            // Actual type of range of entities to create
            const std::string actualType = (type == "TileRangeHorizontal" || type == "TileRangeVertical") ? "Tile" : "Decoration";
            std::string animationName;
            float gx;
            float gy;

            levelSpec >> animationName >> gx >> gy;

            // Range Horizontal
            if (type == "TileRangeHorizontal" || type == "DecorationRangeHorizontal")
            {
                int width;
                levelSpec >> width;

                for (int i = 0; i < width; i++)
                {
                    const int currentGx = gx + i;

                    createStaticEntity(actualType, animationName, currentGx, gy);
                }
            }
            // Range Vertical
            else
            {
                int height;
                levelSpec >> height;

                for (int i = 0; i < height; i++)
                {
                    const int currentGy = gy + i;

                    createStaticEntity(actualType, animationName, gx, currentGy);
                }
            }
            continue;
        }
        else if (type == "Goomba")
        {
            float gx;
            float gy;
            float ad;

            levelSpec >> gx >> gy >> ad;

            createEnemyEntity(type, gx, gy, ad);
            continue;
        }
        else
        {
            std::cout << "Error: " << type << " is not or not yet a supported entity type.\n";
            return;
        }
    }
}

/**
 * Spawns/Re-spawns the player.
 * 
 * Note, does not delete previous player if is re-spawning player.
 * Take this into account when re-spawning the player.
 */
void Scene_Play::spawnPlayer()
{
    auto player = m_entityManager.addEntity("Player");
    player->addComponent<CAnimation>(m_game->assets().getAnimation("MarioStand"), true);
    player->addComponent<CTransform>(gridToCartesianRepresentation(4,7,player));
    player->addComponent<CBoundingBox>(Vec2(64, 64));
    player->addComponent<CInput>();
    player->addComponent<CState>();
    m_player = player;

    m_player->getComponent<CTransform>().acc_y = m_jumpVK.GRAVITY_S;
}

/**
 * NOT YET IMPLEMENTED.
 */
void Scene_Play::spawnBullet(std::shared_ptr<Entity> entity)
{
}

Scene_Play::Scene_Play(GameEngine * gameEngine, const std::string & levelPath)
    : Scene(gameEngine)
{
    m_levelPath = "bin/texts/level1.txt";
    init();
}

/**
 * Renders the next game frame.
 * 
 * Before it renders the next frame, it updates entities, and runs the game systems.
 */
void Scene_Play::update()
{
    m_entityManager.update();

    if (!m_player->isActive())
    {
        spawnPlayer();
    }

    // Call systems that calculate state of entities
    sEnemyState();
    sPlayerState();

    // Call systems that depend on entity state
    sAnimation();
    sMovement();
    sCollision();

    // Render the frame to the screen
    sRender();
}

/**
 * Player animation system.
 * 
 * Note, it just checks what animation player needs for current frame.
 * It does not update the a player's animation (calls <animation>.update()).
 */
void Scene_Play::sPlayerAnimation()
{
    const CState& cState = m_player->getComponent<CState>();
    CTransform& cTransform = m_player->getComponent<CTransform>();
    CAnimation& cAnimation = m_player->getComponent<CAnimation>();
    std::string nextAnimation = "";

    // Figure out animation for the current frame
    if (cState.isGrounded) // grounded
    {
        if (cState.acceleration == Acceleration::ZERO && cTransform.velocity.x == 0) // not moving
        {
            nextAnimation = "MarioStand";
        }
        else if (cState.isSkidding) // skidding
        {
            nextAnimation = "MarioSkid";
        }
        else if (cTransform.velocity.x > m_groundedHK.MAX_WALK_SPEED || cTransform.velocity.x < -m_groundedHK.MAX_WALK_SPEED) // running
        {
            nextAnimation = "MarioRun";
        }
        else // walking
        {
            nextAnimation = "MarioWalk";
        }
    }
    else // airborne
    {
        nextAnimation = "MarioAir";
    }

    const std::string currentAnimation = cAnimation.animation.getName();
    
    // Only change animations if previous animation is different from this frame's animation
    if (currentAnimation != nextAnimation)
    {
        Animation next = m_game->assets().getAnimation(nextAnimation);
        if ((nextAnimation == "MarioRun" && currentAnimation == "MarioWalk") || (nextAnimation == "MarioWalk" && currentAnimation == "MarioRun"))
        {
            // For a smooth transition from walking to running, and running to walking
            // (Both use exact same animation texture, but with different animation speeds.)
            next.setCurrentAnimationFrame(cAnimation.animation.getCurrentAnimationFrameIndex());
        }
        cAnimation.animation = next;
        cAnimation.repeat = true;
    }

    // Make animation face same direction player is facing
    if ((cTransform.scale.x < 0 && cState.facingDir == Direction::RIGHT) || (cTransform.scale.x > 0 && cState.facingDir == Direction::LEFT))
    {
        cTransform.scale.x *= -1;
    }
}

/**
 * The animation system.
 */
void Scene_Play::sAnimation()
{
    sPlayerAnimation();

    // Update animations for all entities (includes player animation)
    for (auto e: m_entityManager.getEntities())
    {
        e->getComponent<CAnimation>().animation.update();
    }

    // Animations are short lived entities, who die when their animation is over
    for (auto e : m_entityManager.getEntities("Animation"))
    {
        if (e->getComponent<CAnimation>().animation.hasEnded())
        {
            e->destroy();
        }
    }
}

/**
 * Player state system.
 */
void Scene_Play::sPlayerState()
{
    CState& cState = m_player->getComponent<CState>();
    const CInput& cInput = m_player->getComponent<CInput>();
    const CTransform& cTransform = m_player->getComponent<CTransform>();

    const bool isAirborne      = !cState.isGrounded;
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
    bool isSkidding          = false;
    Direction newFacingDirection = cState.facingDir;

    if (isAirborne)
    {
        const bool isChangingMovementDirection    = (isMovingLeft && (isPressingRight && !isPressingLeft)) || (isMovingRight && (isPressingLeft && !isPressingRight)); // turning only, doesn't include stopping
        const bool isAboveInitialSpeedThresholdForVel = (cState.initialJumpXSpeed <= -m_airborneHK.INITIAL_SPEED_THRESHOLD_FOR_VEL || cState.initialJumpXSpeed >= m_airborneHK.INITIAL_SPEED_THRESHOLD_FOR_VEL);
        const double maxXSpeed                    = isAboveInitialSpeedThresholdForVel ? m_airborneHK.ABOVE_IST_SPEED_LIMIT_VEL : m_airborneHK.BELLOW_ISP_SPEED_LIMIT_VEL;
        const bool isBellowMaxSpeed               = (cTransform.velocity.x < maxXSpeed) && (cTransform.velocity.x > -maxXSpeed);

        isDeceleratingLeft  = (isMovingRight && isChangingMovementDirection);
        isDeceleratingRight = (isMovingLeft && isChangingMovementDirection);
        isAcceleratingLeft  = (isPressingLeft && !isPressingRight) && (isMovingLeft || isStandingStill) && isBellowMaxSpeed;
        isAcceleratingRight = isPressingRight && !isPressingLeft && (isStandingStill || isMovingRight) && isBellowMaxSpeed;
    }
    else // grounded
    {
        const bool isRunning                    = cInput.B;
        const bool isAtMaxWalkSpeed             = cTransform.velocity.x == m_groundedHK.MAX_WALK_SPEED || cTransform.velocity.x == -m_groundedHK.MAX_WALK_SPEED;
        const bool isAtMaxRunSpeed              = cTransform.velocity.x == m_groundedHK.MAX_RUN_SPEED || cTransform.velocity.x == -m_groundedHK.MAX_RUN_SPEED;
        const bool isWalkingButPastMaxWalkSpeed = (cTransform.velocity.x > m_groundedHK.MAX_WALK_SPEED || cTransform.velocity.x < -m_groundedHK.MAX_WALK_SPEED) && !isRunning;
        const bool isChangingMovementDirection  = (isMovingLeft && (isPressingRight || !isPressingLeft)) || (isMovingRight && (isPressingLeft || !isPressingRight)); // stopping, or turning
        const bool isSkiddingInPreviousFrame    = cState.isSkidding;
        
        isDeceleratingLeft  = (isMovingRight && (isChangingMovementDirection || isWalkingButPastMaxWalkSpeed));
        isDeceleratingRight = (isMovingLeft && (isChangingMovementDirection || isWalkingButPastMaxWalkSpeed));
        isAcceleratingLeft  = (isPressingLeft && !isPressingRight) && (isMovingLeft || isStandingStill) && (!isAtMaxWalkSpeed || isRunning) && !isAtMaxRunSpeed && !isWalkingButPastMaxWalkSpeed;
        isAcceleratingRight = isPressingRight && !isPressingLeft && (isStandingStill || isMovingRight) && (!isAtMaxWalkSpeed || isRunning) && !isAtMaxRunSpeed && !isWalkingButPastMaxWalkSpeed;
        isSkidding          = ((isMovingRight && isPressingLeft && !isPressingRight) || (isMovingLeft && isPressingRight && !isPressingLeft)) || ((isDeceleratingLeft || isDeceleratingRight) && isSkiddingInPreviousFrame);

        if (isSkidding)
        {
            if (isDeceleratingLeft)
            {
                newFacingDirection = Direction::LEFT;
            }
            else // decelerating right
            {
                newFacingDirection = Direction::RIGHT;
            }
        }
        else if (cTransform.velocity.x < 0)
        {
            newFacingDirection = Direction::LEFT;
        }
        else if (cTransform.velocity.x > 0)
        {
            newFacingDirection = Direction::RIGHT;
        }
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

    cState.isSkidding = isSkidding;
    cState.facingDir = newFacingDirection;
}

/**
 * Player airborne movement system.
 */
void Scene_Play::sPlayerAirBorneMovement()
{
    CTransform& cTransform  = m_player->getComponent<CTransform>();
    const CInput& cInput    = m_player->getComponent<CInput>();
    const CState& cState    = m_player->getComponent<CState>();

    const bool isAboveInitialSpeedThresholdForVel = (cState.initialJumpXSpeed <= -m_airborneHK.INITIAL_SPEED_THRESHOLD_FOR_VEL || cState.initialJumpXSpeed >= m_airborneHK.INITIAL_SPEED_THRESHOLD_FOR_VEL);
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

    if (cState.acceleration == Acceleration::ACCELERATING_LEFT)
    {
        cTransform.acc_x = -accelerationX;
    }
    else if (cState.acceleration == Acceleration::ACCELERATING_RIGHT)
    {
        cTransform.acc_x = accelerationX;
    }
    else if (cState.acceleration == Acceleration::DECELERATING_LEFT)
    {
        cTransform.acc_x = -decelerationX;
    }
    else if (cState.acceleration == Acceleration::DECELERATING_RIGHT)
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
    if (cTransform.velocity.y > m_jumpVK.MAX_DOWNWARD_SPEED)
    {
        cTransform.velocity.y = m_jumpVK.RESET_DOWNWARD_SPEED;
    }

    // Step 7: Use velocity to calculate player position
    cTransform.prevPos =  cTransform.pos;
    cTransform.pos += cTransform.velocity;
}

/**
 * Player grounded movement system.
 */
void Scene_Play::sPlayerGroundedMovement()
{
    CInput& cInput         = m_player->getComponent<CInput>();
    CTransform& cTransform = m_player->getComponent<CTransform>();
    CState& cState         = m_player->getComponent<CState>();

    const bool isPressingLeft  = cInput.left;
    const bool isPressingRight = cInput.right;
    const bool isRunning       = cInput.B;
    const bool isWalking       = !cInput.B;
    const bool isSkidding      = cState.isSkidding;
    const bool isMovingRight   = cTransform.velocity.x > 0;
    const bool isMovingLeft    = cTransform.velocity.x < 0;

    // Decelerating: speed is decreasing (going to zero)
    // Accelerating: speed is increasing (going away from zero)
    const bool isDeceleratingLeft  = (cState.acceleration == Acceleration::DECELERATING_LEFT);
    const bool isDeceleratingRight = (cState.acceleration == Acceleration::DECELERATING_RIGHT);
    const bool isAcceleratingLeft  = (cState.acceleration == Acceleration::ACCELERATING_LEFT);
    const bool isAcceleratingRight = (cState.acceleration == Acceleration::ACCELERATING_RIGHT);

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

/**
 * The movement system.
 */
void Scene_Play::sMovement()
{
    // Handle player movement
    if (!m_player->getComponent<CState>().isGrounded)
    {
        sPlayerAirBorneMovement();
    }
    else
    {
        sPlayerGroundedMovement();
    }

    // Handle enemy movement
    for (auto e : m_entityManager.getEntities("Enemy"))
    {
        // Inactive Goombas can't move
        if (!e->getComponent<CEnemy>().isActive)
        {
            continue;
        }

        CTransform& enemyCT = e->getComponent<CTransform>();
        enemyCT.velocity.y += enemyCT.acc_y;
        enemyCT.prevPos = enemyCT.pos;
        enemyCT.pos += enemyCT.velocity;
    }

    // Player fell off the map
    if (m_player->getComponent<CTransform>().pos.y - 64/2 > m_game->window().getSize().y)
    {
        m_player->destroy();
    }
}

/**
 * The enemy state system.
 */
void Scene_Play::sEnemyState()
{
    for (auto goomba : m_entityManager.getEntities("Enemy"))
    {
        // Goombas are activated when player comes within a certain range
        if (goomba->getComponent<CEnemy>().activation_x <= m_player->getComponent<CTransform>().pos.x)
        {
            goomba->getComponent<CEnemy>().isActive = true;
        }
    }
}

/**
 * The player collision system.
 */
void Scene_Play::sPlayerCollision()
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

    // Player-Goomba CD & CR
    for (auto goomba : m_entityManager.getEntities("Enemy"))
    {
        if (!goomba->getComponent<CEnemy>().isActive)
        {
            continue;
        }

        Vec2 overlap = Physics::GetOverlap(m_player, goomba);
        if (Physics::IsCollision(overlap))
        {
            CTransform& playerCT = m_player->getComponent<CTransform>();
            Vec2 prevOverlap = Physics::GetPreviousOverlap(m_player, goomba);
            bool isPlayerStompingGoomba = playerCT.velocity.y > 0 && !m_player->getComponent<CState>().isGrounded;
            if (isPlayerStompingGoomba)
            {
                goomba->destroy();
                playerCT.velocity.y = -m_jumpVK.GOOMBA_STOMP_VELOCITY;
                playerCT.pos.y -= overlap.y;


                // create a dead goomba add it to list "Dead Goombas"
                // place it a original goombas location
                auto stompedGoomba = m_entityManager.addEntity("Animation");
                stompedGoomba->addComponent<CAnimation>(m_game->assets().getAnimation("GoombaDead"), false);
                stompedGoomba->addComponent<CTransform>(goomba->getComponent<CTransform>().pos);
                break;
            }
            else
            {
                m_player->destroy();
                break;
            }
        }
    }
}

/**
 * The enemy collision system.
 */
void Scene_Play::sEnemyCollision()
{
    // Enemy-Tile collisions (detection & resolution)
    for (auto enemy : m_entityManager.getEntities("Enemy"))
    {
        if (!enemy->getComponent<CEnemy>().isActive)
        {
            continue;
        }

        CTransform& enemyCT = enemy->getComponent<CTransform>(); 
        for (auto block : m_entityManager.getEntities("Tile"))
        {
            CTransform& blockCT = block->getComponent<CTransform>();

            Vec2 overlap = Physics::GetOverlap(enemy, block);
            Vec2 prevOverlap = Physics::GetPreviousOverlap(enemy, block);
            if (Physics::IsCollision(overlap))
            {
                CollisionDirection locationBlockWasHit = Physics::GetCollisionDirection(prevOverlap, enemyCT.prevPos, blockCT.pos);

                if (locationBlockWasHit == CollisionDirection::TOP)
                {
                    // Push enemy up
                    enemyCT.pos.y -= overlap.y;
                    enemyCT.velocity.y = 0;
                }
                else if (locationBlockWasHit == CollisionDirection::RIGHT)
                {
                    // push enemy right
                    enemyCT.pos.x += overlap.x;
                    enemyCT.velocity.x *= -1;
                }
                else if (locationBlockWasHit == CollisionDirection::LEFT)
                {
                    // push enemy left
                    enemyCT.pos.x -= overlap.x;
                    enemyCT.velocity.x *= -1;
                }
            }
        }
    }

    // Goomba-Goomba collisions (detection & resolution)
    for (auto goomba1 : m_entityManager.getEntities("Enemy"))
    {
        if (!goomba1->getComponent<CEnemy>().isActive)
        {
            continue;
        }

        for (auto goomba2 : m_entityManager.getEntities("Enemy"))
        {
            if (!goomba2->getComponent<CEnemy>().isActive || goomba1->id() == goomba2->id())
            {
                continue;
            }

            // Note: goombas may have some overlap
            Vec2 overlap = Physics::GetOverlap(goomba1, goomba2);
            if (Physics::IsCollision(overlap))
            {
                CTransform& g1CT = goomba1->getComponent<CTransform>();
                CTransform& g2CT = goomba2->getComponent<CTransform>();
                if (g1CT.pos.x <= g2CT.velocity.x)
                { 
                    // Leftmost goomba walks left
                    g1CT.velocity.x *= (g1CT.velocity.x < 0) ? 1 : -1;
                    // Rightmost goomba walks right
                    g2CT.velocity.x *= (g2CT.velocity.x > 0) ? 1 : -1;

                    g1CT.pos.x -= overlap.x/2;
                    g2CT.pos.x += overlap.x/2;
                }
                else
                {
                    // Leftmost goomba walks left
                    g2CT.velocity.x *= (g2CT.velocity.x < 0) ? 1 : -1;
                    // Rightmost goomba walks right
                    g1CT.velocity.x *= (g1CT.velocity.x > 0) ? 1 : -1;

                    g1CT.pos.x += overlap.x/2;
                    g2CT.pos.x -= overlap.x/2;
                }
            }
        }
    }
}

/**
 * The collision system.
 */
void Scene_Play::sCollision()
{
    sPlayerCollision();
    sEnemyCollision();
}

/**
 * Renders the given entities to the window.
 */
void Scene_Play::sRenderEntities(EntityVec & entities)
{
    sf::RenderWindow & window = m_game->window();

    for (auto e : entities)
    {
        const Vec2 posRelativeToCamera = e->getComponent<CTransform>().pos - m_cameraPosition;
        const Vec2 scale = e->getComponent<CTransform>().scale;
        sf::Sprite & sprite = e->getComponent<CAnimation>().animation.getSprite();

        sprite.setPosition(sf::Vector2f(posRelativeToCamera.x,posRelativeToCamera.y));
        sprite.setScale(sf::Vector2f(scale.x, scale.y));
        window.draw(sprite);
    }
}

/**
 * The render system.
 */
void Scene_Play::sRender()
{
    sf::RenderWindow & window = m_game->window();
    window.clear(sf::Color(97, 126, 248)); 
    
    // Update camera position
    float newCameraPosX = m_player->getComponent<CTransform>().pos.x - window.getSize().x/2;
    if (newCameraPosX < m_cameraPosition.x)
    {
        // Camera shouldn't move backwards
        newCameraPosX = m_cameraPosition.x;
    }
    m_cameraPosition.x = newCameraPosX;

    // Draw entities
    if (m_drawTextures)
    {
        // Rendering order
        sRenderEntities(m_entityManager.getEntities("Decoration"));
        sRenderEntities(m_entityManager.getEntities("Tile"));
        sRenderEntities(m_entityManager.getEntities("Enemy"));
        sRenderEntities(m_entityManager.getEntities("Animation"));
        sRenderEntities(m_entityManager.getEntities("Player"));
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
        const int heightCell = m_gridCellSize.y;
        const int widthCell = m_gridCellSize.x;
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