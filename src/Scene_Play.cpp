#include "Scene_Play.h"
#include "Physics.h"
#include <iostream>
#include <sstream> 
#include <cmath>
#include <fstream>
#include "PhysicsConstants.h"

/**
 * Reloads the level.
 * 
 * Use after player gets killed by enemy or falls off the map.
 */
void Scene_Play::reloadLevel()
{
    m_entityManager = EntityManager();
    loadLevel();
    spawnPlayer();
    m_cameraPosition = Vec2(0.f,0.f);
}

/**
 * Initializes the object. 
 * Should be called once and only once before any other class methods are called.
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
void Scene_Play::createStaticEntity(const std::string& type, const std::string& animation, float gx, float gy)
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
void Scene_Play::createEnemyEntity(const std::string& type, float gx, float gy, float activationDistance)
{
    if (type != "Goomba")
    {
        std::cout << "Error: " << type << " creation is not yet supported!\n";
        return;
    }

    auto e = m_entityManager.addEntity("Enemy");

    // Add components
    e->addComponent<CAnimation>(m_game->assets().getAnimation("GoombaWalk"), true);
    e->addComponent<CBoundingBox>(Vec2(64,64));
    CTransform& goombaCT = e->addComponent<CTransform>(gridToCartesianRepresentation(gx,gy,e));
    CEnemy& goombaCE =  e->addComponent<CEnemy>();

    // Initialize components
    goombaCT.velocity.x = -ENEMY_KINEMATICS::GOOMBA_SPEED;
    goombaCT.acc_y = ENEMY_KINEMATICS::GRAVITY;
    goombaCE.activation_x = (gx - activationDistance) * 64;
    goombaCE.type = EnemyType::GOOMBA;
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
        else if (type == "Koopa")
        {
            float gx;
            float gy;
            float ad;

            levelSpec >> gx >> gy >> ad;

            const Vec2 KOOPA_BB = Vec2(64,92);
            auto koopa = m_entityManager.addEntity("Enemy");
            koopa->addComponent<CEnemy>(EnemyType::KOOPA, false, (gx - ad) * 64);
            koopa->addComponent<CAnimation>(m_game->assets().getAnimation("KoopaWalk"), true);
            koopa->addComponent<CTransform>(gridToCartesianRepresentation(Vec2(gx,gy), KOOPA_BB), Vec2(-ENEMY_KINEMATICS::KOOPA_SPEED, 0), Vec2(1,1), 0, 0, ENEMY_KINEMATICS::GRAVITY);
            koopa->addComponent<CBoundingBox>(KOOPA_BB);

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
    player->addComponent<CBoundingBox>(Vec2(56, 64));
    player->addComponent<CInput>();
    player->addComponent<CState>();
    m_player = player;

    m_player->getComponent<CTransform>().acc_y = AIRBORNE_VERTICAL_KINEMATICS::GRAVITY_S;
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
        reloadLevel();
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
        else if (cTransform.velocity.x > GROUNDED_HORIZONTAL_KINEMATICS::MAX_WALK_SPEED || cTransform.velocity.x < -GROUNDED_HORIZONTAL_KINEMATICS::MAX_WALK_SPEED) // running
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

    for (auto e : m_entityManager.getEntities("Enemy"))
    {
        CTransform& eCT = e->getComponent<CTransform>();
        if (eCT.velocity.x < 0) // for koopas and goombas -1 is facing right, 1 is facing left **sigh**
        {
            eCT.scale.x = 1;
        }
        else if (eCT.velocity.x > 0)
        {
            eCT.scale.x = -1;
        }
    }

    // Animations are short lived entities, who die when their animation is over
    for (auto e : m_entityManager.getEntities("Animation"))
    {
        if (!e->hasComponent<CLifeSpan>() && e->getComponent<CAnimation>().animation.hasEnded())
        {
            e->destroy();
            e->removeComponent<CTransform>();
            e->removeComponent<CAnimation>();
        }

        // TODO: CREATE AND MOVE TO sState
        if (e->hasComponent<CLifeSpan>())
        {
            e->getComponent<CLifeSpan>().lifespan -= 1;

            if (e->getComponent<CLifeSpan>().lifespan == 0)
            {
                e->destroy();
                e->removeComponent<CAnimation>();
                e->removeComponent<CTransform>();
            }
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
        const bool isAboveInitialSpeedThresholdForVel = (cState.initialJumpXSpeed <= -AIRBORNE_HORIZONTAL_KINEMATICS::INITIAL_SPEED_THRESHOLD_FOR_VEL || cState.initialJumpXSpeed >= AIRBORNE_HORIZONTAL_KINEMATICS::INITIAL_SPEED_THRESHOLD_FOR_VEL);
        const double maxXSpeed                    = isAboveInitialSpeedThresholdForVel ? AIRBORNE_HORIZONTAL_KINEMATICS::ABOVE_IST_SPEED_LIMIT_VEL : AIRBORNE_HORIZONTAL_KINEMATICS::BELLOW_ISP_SPEED_LIMIT_VEL;
        const bool isBellowMaxSpeed               = (cTransform.velocity.x < maxXSpeed) && (cTransform.velocity.x > -maxXSpeed);

        isDeceleratingLeft  = (isMovingRight && isChangingMovementDirection);
        isDeceleratingRight = (isMovingLeft && isChangingMovementDirection);
        isAcceleratingLeft  = (isPressingLeft && !isPressingRight) && (isMovingLeft || isStandingStill) && isBellowMaxSpeed;
        isAcceleratingRight = isPressingRight && !isPressingLeft && (isStandingStill || isMovingRight) && isBellowMaxSpeed;
    }
    else // grounded
    {
        const bool isRunning                    = cInput.B;
        const bool isAtMaxWalkSpeed             = cTransform.velocity.x == GROUNDED_HORIZONTAL_KINEMATICS::MAX_WALK_SPEED || cTransform.velocity.x == -GROUNDED_HORIZONTAL_KINEMATICS::MAX_WALK_SPEED;
        const bool isAtMaxRunSpeed              = cTransform.velocity.x == GROUNDED_HORIZONTAL_KINEMATICS::MAX_RUN_SPEED || cTransform.velocity.x == -GROUNDED_HORIZONTAL_KINEMATICS::MAX_RUN_SPEED;
        const bool isWalkingButPastMaxWalkSpeed = (cTransform.velocity.x > GROUNDED_HORIZONTAL_KINEMATICS::MAX_WALK_SPEED || cTransform.velocity.x < -GROUNDED_HORIZONTAL_KINEMATICS::MAX_WALK_SPEED) && !isRunning;
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

    const bool isAboveInitialSpeedThresholdForVel = (cState.initialJumpXSpeed <= -AIRBORNE_HORIZONTAL_KINEMATICS::INITIAL_SPEED_THRESHOLD_FOR_VEL || cState.initialJumpXSpeed >= AIRBORNE_HORIZONTAL_KINEMATICS::INITIAL_SPEED_THRESHOLD_FOR_VEL);
    const bool isAboveCurrentSpeedThresholdForAcc = (cTransform.velocity.x <= -AIRBORNE_HORIZONTAL_KINEMATICS::CURRENT_SPEED_THRESHOLD_FOR_ACC || cTransform.velocity.x >= AIRBORNE_HORIZONTAL_KINEMATICS::CURRENT_SPEED_THRESHOLD_FOR_ACC);
    const bool isAboveInitialSpeedThresholdForAcc = (cState.initialJumpXSpeed <= -AIRBORNE_HORIZONTAL_KINEMATICS::INITIAL_SPEED_THRESHOLD_FOR_ACC || cState.initialJumpXSpeed >= AIRBORNE_HORIZONTAL_KINEMATICS::INITIAL_SPEED_THRESHOLD_FOR_ACC);

    // Step 1: Figure out X acceleration for current frame
    double accelerationX = 0;
    if (isAboveCurrentSpeedThresholdForAcc)
    {
        accelerationX = AIRBORNE_HORIZONTAL_KINEMATICS::ABOVE_CST_ACC;
    }
    else
    {
        accelerationX = AIRBORNE_HORIZONTAL_KINEMATICS::BELOW_CST_ACC;
    }
    double decelerationX = 0;
    if (isAboveCurrentSpeedThresholdForAcc)
    {
        decelerationX = AIRBORNE_HORIZONTAL_KINEMATICS::ABOVE_CST_DEC;
    }
    else if (isAboveInitialSpeedThresholdForAcc)
    {
        decelerationX = AIRBORNE_HORIZONTAL_KINEMATICS::ABOVE_IST_DEC;
    }
    else
    {
        decelerationX = AIRBORNE_HORIZONTAL_KINEMATICS::BELOW_IST_DEC;
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
        if (cTransform.velocity.x > AIRBORNE_HORIZONTAL_KINEMATICS::ABOVE_IST_SPEED_LIMIT_VEL)
        {
            cTransform.velocity.x = AIRBORNE_HORIZONTAL_KINEMATICS::ABOVE_IST_SPEED_LIMIT_VEL;
        }
        else if (cTransform.velocity.x < -AIRBORNE_HORIZONTAL_KINEMATICS::ABOVE_IST_SPEED_LIMIT_VEL)
        {
            cTransform.velocity.x = -AIRBORNE_HORIZONTAL_KINEMATICS::ABOVE_IST_SPEED_LIMIT_VEL;
        }
    }
    else
    {
        if (cTransform.velocity.x > AIRBORNE_HORIZONTAL_KINEMATICS::BELLOW_ISP_SPEED_LIMIT_VEL)
        {
            cTransform.velocity.x = AIRBORNE_HORIZONTAL_KINEMATICS::BELLOW_ISP_SPEED_LIMIT_VEL;
        }
        else if (cTransform.velocity.x < -AIRBORNE_HORIZONTAL_KINEMATICS::BELLOW_ISP_SPEED_LIMIT_VEL)
        {
            cTransform.velocity.x = -AIRBORNE_HORIZONTAL_KINEMATICS::BELLOW_ISP_SPEED_LIMIT_VEL;
        }
    }
    
    const bool isPressingJump                         = cInput.A;
    const bool hadReducedGravity                      = (cTransform.acc_y == AIRBORNE_VERTICAL_KINEMATICS::REDUCED_GRAVITY_S) || (cTransform.acc_y == AIRBORNE_VERTICAL_KINEMATICS::REDUCED_GRAVITY_M);
    const bool isFalling                              = (cTransform.velocity.y >= 0);
    const bool isJustStartingNormalGravityPhaseOfJump = hadReducedGravity && (!isPressingJump || isFalling);

    // Step 4: Figure out Y acceleration
    if (isJustStartingNormalGravityPhaseOfJump)
    {
        if (cTransform.acc_y == AIRBORNE_VERTICAL_KINEMATICS::REDUCED_GRAVITY_S)
        {
            cTransform.acc_y = AIRBORNE_VERTICAL_KINEMATICS::GRAVITY_S;
        }
        else if (cTransform.acc_y == AIRBORNE_VERTICAL_KINEMATICS::REDUCED_GRAVITY_M)
        {
            cTransform.acc_y = AIRBORNE_VERTICAL_KINEMATICS::GRAVITY_M;
        }
        else
        {
            cTransform.acc_y = AIRBORNE_VERTICAL_KINEMATICS::GRAVITY_L;
        }
    }

    // Step 5: Use Y acceleration to calculate Y velocity
    cTransform.velocity.y += cTransform.acc_y;

    // Step 6: Apply speed limits or exception for Y velocity
    if (cTransform.velocity.y > AIRBORNE_VERTICAL_KINEMATICS::MAX_DOWNWARD_SPEED)
    {
        cTransform.velocity.y = AIRBORNE_VERTICAL_KINEMATICS::RESET_DOWNWARD_SPEED;
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
        accelerationX = GROUNDED_HORIZONTAL_KINEMATICS::RUN_ACC;
    }
    else
    {
        accelerationX = GROUNDED_HORIZONTAL_KINEMATICS::WALK_ACC;
    }
    double decelerationX = 0;
    if (isSkidding) 
    {
        decelerationX = GROUNDED_HORIZONTAL_KINEMATICS::SKID_DEC;
    }
    else
    {
        decelerationX = GROUNDED_HORIZONTAL_KINEMATICS::RELEASE_DEC;
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

    const bool isPastMaxWalkSpeed      = cTransform.velocity.x > GROUNDED_HORIZONTAL_KINEMATICS::MAX_WALK_SPEED || cTransform.velocity.x < -GROUNDED_HORIZONTAL_KINEMATICS::MAX_WALK_SPEED;
    const bool isPastMaxRunSpeed       = cTransform.velocity.x > GROUNDED_HORIZONTAL_KINEMATICS::MAX_RUN_SPEED || cTransform.velocity.x < -GROUNDED_HORIZONTAL_KINEMATICS::MAX_RUN_SPEED;
    const bool isBellowMinWalkSpeed    = cTransform.velocity.x < GROUNDED_HORIZONTAL_KINEMATICS::MIN_WALK_SPEED && cTransform.velocity.x > -GROUNDED_HORIZONTAL_KINEMATICS::MIN_WALK_SPEED;
    const bool isBellowTurnAroundSpeed = cTransform.velocity.x < GROUNDED_HORIZONTAL_KINEMATICS::SKID_TURNAROUND_SPEED && cTransform.velocity.x > -GROUNDED_HORIZONTAL_KINEMATICS::SKID_TURNAROUND_SPEED;

    // Step 3: Apply speed limits or exception for X velocity
    // Mario is walking right and past max walk speed
    if (isPastMaxWalkSpeed && isAcceleratingRight && isWalking)
    {
        cTransform.velocity.x = GROUNDED_HORIZONTAL_KINEMATICS::MAX_WALK_SPEED;
    }
    // Mario is walking left and past max walk speed
    else if (isPastMaxWalkSpeed && isAcceleratingLeft && isWalking)
    {
        cTransform.velocity.x = -GROUNDED_HORIZONTAL_KINEMATICS::MAX_WALK_SPEED;
    }
    // Mario is running right and past max run speed
    else if (isPastMaxRunSpeed && isAcceleratingRight && isRunning)
    {
        cTransform.velocity.x = GROUNDED_HORIZONTAL_KINEMATICS::MAX_RUN_SPEED;   
    }
    // Mario is running left and past max run speed
    else if (isPastMaxRunSpeed && isAcceleratingLeft && isRunning)
    {
        cTransform.velocity.x = -GROUNDED_HORIZONTAL_KINEMATICS::MAX_RUN_SPEED;   
    }
    // Mario is accelerating right and is bellow min x speed
    else if (isBellowMinWalkSpeed && isAcceleratingRight)
    {
        cTransform.velocity.x = GROUNDED_HORIZONTAL_KINEMATICS::MIN_WALK_SPEED;
    }
    // Mario is accelerating left and is bellow min x speed
    else if (isBellowMinWalkSpeed && isAcceleratingLeft)
    {
        cTransform.velocity.x = -GROUNDED_HORIZONTAL_KINEMATICS::MIN_WALK_SPEED;
    }
    // Mario is decelerating and is bellow min x speed
    else if ((isBellowMinWalkSpeed || (isBellowTurnAroundSpeed && isSkidding)) && (isDeceleratingLeft || isDeceleratingRight))
    {
        cTransform.velocity.x = 0;
    }

    const double xSpeed                  = cTransform.velocity.x;
    const bool canJump                   = cInput.canJump;
    const bool isPressingJump            = cInput.A;
    const bool isAtSmallHorizontalSpeed  = (AIRBORNE_VERTICAL_KINEMATICS::SMALL_SPEED_THRESHOLD > xSpeed) && (-AIRBORNE_VERTICAL_KINEMATICS::SMALL_SPEED_THRESHOLD < xSpeed);
    const bool isAtMediumHorizontalSpeed = (AIRBORNE_VERTICAL_KINEMATICS::MEDIUM_SPEED_THRESHOLD >= xSpeed) && (-AIRBORNE_VERTICAL_KINEMATICS::MEDIUM_SPEED_THRESHOLD <= xSpeed) && (!isAtSmallHorizontalSpeed);
    const bool isJustStartingJump        = canJump && isPressingJump;

    // Step 4: Check if player is about to jump
    if (isJustStartingJump)
    {
        if (isAtSmallHorizontalSpeed)
        {
            cTransform.acc_y = AIRBORNE_VERTICAL_KINEMATICS::REDUCED_GRAVITY_S;
        }
        else if (isAtMediumHorizontalSpeed)
        {
            cTransform.acc_y = AIRBORNE_VERTICAL_KINEMATICS::REDUCED_GRAVITY_M;
        }
        else
        {
            cTransform.acc_y = AIRBORNE_VERTICAL_KINEMATICS::REDUCED_GRAVITY_L;
        }
    }

    // Step 5: Use Y acceleration to calculate Y velocity
    cTransform.velocity.y += cTransform.acc_y;

    // Step 6: Apply speed limits or exception for Y velocity
    if (isJustStartingJump)
    {
        if (isAtSmallHorizontalSpeed)
        {
            cTransform.velocity.y = -AIRBORNE_VERTICAL_KINEMATICS::INITIAL_VELOCITY_S;
        }
        else if (isAtMediumHorizontalSpeed)
        {
            cTransform.velocity.y = -AIRBORNE_VERTICAL_KINEMATICS::INITIAL_VELOCITY_M;
        }
        else
        {
            cTransform.velocity.y = -AIRBORNE_VERTICAL_KINEMATICS::INITIAL_VELOCITY_L;
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

    // Handle animation movement
    for (auto e : m_entityManager.getEntities("Animation"))
    {
        if (!e->hasComponent<CTransform>())
        {
            continue;
        }

        CTransform& animationCT = e->getComponent<CTransform>();

        animationCT.velocity.y += animationCT.acc_y;

        if (animationCT.velocity.y > AIRBORNE_VERTICAL_KINEMATICS::MAX_DOWNWARD_SPEED)
        {
            animationCT.velocity.y = AIRBORNE_VERTICAL_KINEMATICS::MAX_DOWNWARD_SPEED;
        }

        animationCT.pos += animationCT.velocity;
        animationCT.angle += animationCT.angularVel;
    }

    // Player fell off the map
    if (m_player->getComponent<CTransform>().pos.y - 64/2 > m_game->window().getSize().y)
    {
        m_player->destroy();
    }

    // Player can not move outside of camera
    if (m_player->getComponent<CTransform>().pos.x - m_player->getComponent<CBoundingBox>().halfSize.x < m_cameraPosition.x)
    {
        m_player->getComponent<CTransform>().pos.x = m_player->getComponent<CBoundingBox>().halfSize.x + m_cameraPosition.x;
    }
}

/**
 * The enemy state system.
 */
void Scene_Play::sEnemyState()
{
    for (auto enemy : m_entityManager.getEntities("Enemy"))
    {
        if (enemy->hasComponent<CLifeSpan>())
        {
            enemy->getComponent<CLifeSpan>().lifespan -= 1;
        }

        // Goombas are activated when player comes within a certain range
        if (enemy->getComponent<CEnemy>().activation_x <= m_player->getComponent<CTransform>().pos.x)
        {
            enemy->getComponent<CEnemy>().isActive = true;
        }

        if (enemy->getComponent<CEnemy>().type == EnemyType::KOOPA)
        {
            if (enemy->hasComponent<CLifeSpan>() && enemy->getComponent<CLifeSpan>().lifespan == 0) // Koopa woke up (out of shell)
            {
                const Vec2 KOOPA_BB = Vec2(64,92);
                const Vec2 EMPTY_SHELL_BB = Vec2(64,64);
                enemy->removeComponent<CLifeSpan>();
                enemy->getComponent<CAnimation>().animation = m_game->assets().getAnimation("KoopaWalk");
                enemy->getComponent<CTransform>().velocity.x = enemy->getComponent<CTransform>().scale.x < 0 ? ENEMY_KINEMATICS::KOOPA_SPEED : -ENEMY_KINEMATICS::KOOPA_SPEED;
                enemy->addComponent<CBoundingBox>(KOOPA_BB);
                enemy->getComponent<CTransform>().pos.y -= KOOPA_BB.y - EMPTY_SHELL_BB.y;
            }
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

            auto coin = m_entityManager.addEntity("Animation");
            coin->addComponent<CLifeSpan>(50,0);
            coin->addComponent<CTransform>(Vec2(bottomHitBlock->getComponent<CTransform>().pos.x, bottomHitBlock->getComponent<CTransform>().pos.y - bottomHitBlock->getComponent<CBoundingBox>().size.y * 1.25));
            coin->addComponent<CAnimation>(m_game->assets().getAnimation("CoinBlink"), true);

            bottomHitBlock->destroy();
        }
        else if (bottomHitBlock->getComponent<CAnimation>().animation.getName() == "Brick")
        {
            bottomHitBlock->destroy();

            CTransform& hitBlockCT = bottomHitBlock->getComponent<CTransform>();
            CBoundingBox hitBlockBB = bottomHitBlock->getComponent<CBoundingBox>();

            {
                auto brokenBrickTL = m_entityManager.addEntity("Animation");
                Vec2 pos (hitBlockCT.pos.x - hitBlockBB.halfSize.x/2, hitBlockCT.pos.y - hitBlockBB.halfSize.y/2);
                Vec2 vel (-GROUNDED_HORIZONTAL_KINEMATICS::MAX_WALK_SPEED, -AIRBORNE_VERTICAL_KINEMATICS::INITIAL_VELOCITY_L * 1.5);
                Vec2 scale (0.5f, 0.5f);
                float angle = 45;
                float angularVel = AIRBORNE_VERTICAL_KINEMATICS::GRAVITY_L * 4;
                brokenBrickTL->addComponent<CTransform>(pos, vel, scale, angle, angularVel, AIRBORNE_VERTICAL_KINEMATICS::GRAVITY_L);
                brokenBrickTL->addComponent<CAnimation>(m_game->assets().getAnimation("BrokenBrick"), false);
            }

            {
                auto brokenBrickTR = m_entityManager.addEntity("Animation");
                Vec2 pos (hitBlockCT.pos.x + hitBlockBB.halfSize.x/2, hitBlockCT.pos.y - hitBlockBB.halfSize.y/2);
                Vec2 vel (GROUNDED_HORIZONTAL_KINEMATICS::MAX_WALK_SPEED, -AIRBORNE_VERTICAL_KINEMATICS::INITIAL_VELOCITY_L * 1.5);
                Vec2 scale (0.5f, 0.5f);
                float angle = -45;
                float angularVel = AIRBORNE_VERTICAL_KINEMATICS::GRAVITY_L * 4;
                brokenBrickTR->addComponent<CTransform>(pos, vel, scale, angle, angularVel, AIRBORNE_VERTICAL_KINEMATICS::GRAVITY_L);
                brokenBrickTR->addComponent<CAnimation>(m_game->assets().getAnimation("BrokenBrick"), false);
            }

            {
                auto brokenBrickBL = m_entityManager.addEntity("Animation");
                Vec2 pos (hitBlockCT.pos.x - hitBlockBB.halfSize.x/2, hitBlockCT.pos.y + hitBlockBB.halfSize.y/2);
                Vec2 vel (-GROUNDED_HORIZONTAL_KINEMATICS::MAX_WALK_SPEED * 1.5, -AIRBORNE_VERTICAL_KINEMATICS::INITIAL_VELOCITY_L);
                Vec2 scale (0.5f, 0.5f);
                float angle = 45;
                float angularVel = AIRBORNE_VERTICAL_KINEMATICS::GRAVITY_L * 4;
                brokenBrickBL->addComponent<CTransform>(pos, vel, scale, angle, angularVel, AIRBORNE_VERTICAL_KINEMATICS::GRAVITY_L);
                brokenBrickBL->addComponent<CAnimation>(m_game->assets().getAnimation("BrokenBrick"), false);
            }

            {
                auto brokenBrickBL = m_entityManager.addEntity("Animation");
                Vec2 pos (hitBlockCT.pos.x + hitBlockBB.halfSize.x/2, hitBlockCT.pos.y + hitBlockBB.halfSize.y/2);
                Vec2 vel (GROUNDED_HORIZONTAL_KINEMATICS::MAX_WALK_SPEED * 1.5, -AIRBORNE_VERTICAL_KINEMATICS::INITIAL_VELOCITY_L);
                Vec2 scale (0.5f, 0.5f);
                float angle = -45;
                float angularVel = AIRBORNE_VERTICAL_KINEMATICS::GRAVITY_L * 4;
                brokenBrickBL->addComponent<CTransform>(pos, vel, scale, angle, angularVel, AIRBORNE_VERTICAL_KINEMATICS::GRAVITY_L);
                brokenBrickBL->addComponent<CAnimation>(m_game->assets().getAnimation("BrokenBrick"), false);
            }
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
    for (auto enemy : m_entityManager.getEntities("Enemy"))
    {
        if (!enemy->getComponent<CEnemy>().isActive)
        {
            continue;
        }

        Vec2 overlap = Physics::GetOverlap(m_player, enemy);
        if (Physics::IsCollision(overlap))
        {
            CTransform& playerCT = m_player->getComponent<CTransform>();
            Vec2 prevOverlap = Physics::GetPreviousOverlap(m_player, enemy);
            bool isStomp = playerCT.velocity.y > 0 && !m_player->getComponent<CState>().isGrounded;

            if (isStomp)
            {
                playerCT.velocity.y = -ENEMY_KINEMATICS::STOMP_SPEED;
                playerCT.pos.y -= overlap.y;

                if (enemy->getComponent<CEnemy>().type == EnemyType::GOOMBA)
                {
                    enemy->destroy();
                    // create a dead goomba add it to list "Dead Goombas"
                    // place it a original goombas location
                    auto stompedGoomba = m_entityManager.addEntity("Animation");
                    stompedGoomba->addComponent<CAnimation>(m_game->assets().getAnimation("GoombaDead"), false);
                    stompedGoomba->addComponent<CTransform>(enemy->getComponent<CTransform>().pos);
                    break;
                }
                else // Koopa
                {
                    if (enemy->hasComponent<CLifeSpan>()) // Koopa is in shell
                    {
                        enemy->destroy();
                    }
                    else
                    {
                        const Vec2 EMPTY_SHELL_BB = Vec2(64,64);
                        enemy->getComponent<CAnimation>().animation = m_game->assets().getAnimation("KoopaShell");
                        enemy->getComponent<CTransform>().velocity.x = 0;
                        enemy->addComponent<CLifeSpan>(100,0);
                        enemy->addComponent<CBoundingBox>(EMPTY_SHELL_BB);
                    }
                }
            }
            else
            {
                if (enemy->getComponent<CEnemy>().type == EnemyType::KOOPA && enemy->hasComponent<CLifeSpan>()) // koopa in shell and not moving
                {
                    enemy->removeComponent<CLifeSpan>();
                    if (enemy->getComponent<CTransform>().pos.x < m_player->getComponent<CTransform>().pos.x)
                    {
                        enemy->getComponent<CTransform>().velocity.x = -ENEMY_KINEMATICS::SHELL_SPEED;
                        enemy->getComponent<CTransform>().pos.x -= overlap.x;
                    }
                    else
                    {
                        enemy->getComponent<CTransform>().velocity.x = ENEMY_KINEMATICS::SHELL_SPEED;
                        enemy->getComponent<CTransform>().pos.x += overlap.x;
                    }
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

    // Enemy-Enemy collisions (detection & resolution)
    for (auto enemy1 : m_entityManager.getEntities("Enemy"))
    {
        if (!enemy1->getComponent<CEnemy>().isActive || !enemy1->isActive()) // enemy can't move yet, or was killed
        {
            continue;
        }

        for (auto enemy2 : m_entityManager.getEntities("Enemy"))
        {
            if (!enemy2->getComponent<CEnemy>().isActive || enemy1->id() == enemy2->id() || !enemy2->isActive()) // enemy can't move yet, was killed, or is same enemy
            {
                continue;
            }

            // Note: goombas may have some overlap
            Vec2 overlap = Physics::GetOverlap(enemy1, enemy2);
            if (Physics::IsCollision(overlap))
            {
                // if enemy1 is MKS and enemy2 is NOT MKS
                    // what ever it is it gets killed
                    // continue
                // if enemy1 is not MKS and enemy2 is MKS
                    // ?
                    // 2 options
                        // kill enemy1 and break
                // else DO THE FOLLOWING

                // Moving koopa shell (MKS)
                CTransform& e1CT = enemy1->getComponent<CTransform>();
                CTransform& e2CT = enemy2->getComponent<CTransform>();
                const bool isEnemy1MKS = enemy1->getComponent<CEnemy>().type == EnemyType::KOOPA && enemy1->getComponent<CAnimation>().animation.getName() == "KoopaShell" && enemy1->getComponent<CTransform>().velocity.x != 0;
                const bool isEnemy2MKS = enemy2->getComponent<CEnemy>().type == EnemyType::KOOPA && enemy2->getComponent<CAnimation>().animation.getName() == "KoopaShell" && enemy2->getComponent<CTransform>().velocity.x != 0;

                if (isEnemy1MKS && !isEnemy2MKS) // enemy1 is MKS and hit and killed enemy2
                {
                    // e2 is killed
                    // turn it into animation
                        // same place
                        // same animation
                        // e1 (MKS) came from right
                            // throw e2 animation to right
                            // make it spin clockwise
                        // else came from left
                            // throw e2 animation to left
                            // make it spin counter cc
                    // remove e2 animation (so it doesn't get rendered)
                    auto e2Animation = m_entityManager.addEntity("Animation");
                    Vec2 speed = Vec2(e1CT.velocity.x * -1, -AIRBORNE_VERTICAL_KINEMATICS::INITIAL_VELOCITY_L);
                    float angularSpeed = e1CT.pos.x < e2CT.pos.x ? -10 : 10; // ccc if MKS came from left, else came from right so cc 
                    e2Animation->addComponent<CTransform>(e2CT.pos, speed, Vec2(1,1), 0, angularSpeed, ENEMY_KINEMATICS::GRAVITY);
                    e2Animation->addComponent<CAnimation>();
                    e2Animation->getComponent<CAnimation>().animation = enemy2->getComponent<CAnimation>().animation;
                    e2Animation->addComponent<CLifeSpan>(100, 0);

                    enemy2->destroy();
                    enemy2->removeComponent<CAnimation>();
                }
                else if (isEnemy2MKS && !isEnemy1MKS) // enemy2 is MKS and hit and killed enemy1
                {
                    auto e1Animation = m_entityManager.addEntity("Animation");
                    Vec2 speed = Vec2(e2CT.velocity.x * -1, -AIRBORNE_VERTICAL_KINEMATICS::INITIAL_VELOCITY_L);
                    float angularSpeed = e2CT.pos.x < e1CT.pos.x ? -10 : 10; // ccc if MKS came from left, else came from right so cc 
                    e1Animation->addComponent<CTransform>(e1CT.pos, speed, Vec2(1,1), 0, angularSpeed, ENEMY_KINEMATICS::GRAVITY);
                    e1Animation->addComponent<CAnimation>();
                    e1Animation->getComponent<CAnimation>().animation = enemy1->getComponent<CAnimation>().animation;
                    e1Animation->addComponent<CLifeSpan>(100, 0);

                    enemy1->destroy();
                    enemy1->removeComponent<CAnimation>();
                }
                else // neither is MKS
                {
                    if (e1CT.pos.x <= e2CT.velocity.x)
                    { 
                        // Leftmost goomba walks left
                        e1CT.velocity.x *= (e1CT.velocity.x < 0) ? 1 : -1;
                        // Rightmost goomba walks right
                        e2CT.velocity.x *= (e2CT.velocity.x > 0) ? 1 : -1;

                        e1CT.pos.x -= overlap.x/2;
                        e2CT.pos.x += overlap.x/2;

                    }
                    else
                    {
                        // Leftmost goomba walks left
                        e2CT.velocity.x *= (e2CT.velocity.x < 0) ? 1 : -1;
                        // Rightmost goomba walks right
                        e1CT.velocity.x *= (e1CT.velocity.x > 0) ? 1 : -1;

                        e1CT.pos.x += overlap.x/2;
                        e2CT.pos.x -= overlap.x/2;
                    }
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
        if (!e->hasComponent<CAnimation>())
        {
            continue;
        }

        Vec2 cameraScreenSize = Vec2(m_game->window().getSize().x, m_game->window().getSize().y);
        Vec2 cameraCenterPos = m_cameraPosition + cameraScreenSize/2; // points to the center of the screen

        const Vec2 overlap = Physics::GetOverLap(cameraCenterPos, e->getComponent<CTransform>().pos, cameraScreenSize/2, e->getComponent<CAnimation>().animation.getSize()/2);
        if (!Physics::IsCollision(overlap)) // Cull entity
        {
            continue;
        }

        const Vec2 posRelativeToCamera = e->getComponent<CTransform>().pos - m_cameraPosition;
        const Vec2 scale = e->getComponent<CTransform>().scale;
        sf::Sprite & sprite = e->getComponent<CAnimation>().animation.getSprite();

        sprite.setPosition(sf::Vector2f(posRelativeToCamera.x,posRelativeToCamera.y));
        sprite.setScale(sf::Vector2f(scale.x, scale.y));
        sprite.setRotation(e->getComponent<CTransform>().angle);
        window.draw(sprite);
    }
}

/**
 * Renders bounding boxes for all entities.
 */
void Scene_Play::sRenderBoundingBoxes()
{
    sf::RenderWindow & window = m_game->window();

    for (auto e : m_entityManager.getEntities())
    {
        // WHAT IF E IS DEAD (INACTIVE)?

        Vec2 cameraScreenSize = Vec2(m_game->window().getSize().x, m_game->window().getSize().y);
        Vec2 cameraCenterPos = m_cameraPosition + cameraScreenSize/2; // points to the center of the screen

        const Vec2 overlap = Physics::GetOverLap(cameraCenterPos, e->getComponent<CTransform>().pos, cameraScreenSize/2, e->getComponent<CAnimation>().animation.getSize()/2);
        if (!Physics::IsCollision(overlap)) // Cull entity
        {
            continue;
        }

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

/**
 * Renders debugging grid.
 */
void Scene_Play::sRenderDebugGrid()
{
    sf::RenderWindow & window = m_game->window();

    const int heightWindow = window.getSize().y;
    const int widthWindow = window.getSize().x;
    const int heightCell = m_gridCellSize.y;
    const int widthCell = m_gridCellSize.x;
    const int MAP_WIDTH_BLOCKS = 300;

    const int startingColumn = floor(m_cameraPosition.x/64.f);
    const int endingColumn = startingColumn + ceil(widthWindow/64.f) + 1; // off by one errors, smh...

    // Draw grid vertical lines
    const int verticalLines = ceil((float) widthWindow / widthCell);
    for (int i = startingColumn; i < endingColumn; i++) 
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
    for (int gx = startingColumn; gx < endingColumn; gx++)
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

    if (m_drawTextures)
    {
        // Rendering order
        sRenderEntities(m_entityManager.getEntities("Decoration"));
        sRenderEntities(m_entityManager.getEntities("Tile"));
        sRenderEntities(m_entityManager.getEntities("Enemy"));
        sRenderEntities(m_entityManager.getEntities("Animation"));
        sRenderEntities(m_entityManager.getEntities("Player"));
    }
    if (m_drawCollision)
    {
        sRenderBoundingBoxes();
    }
    if (m_drawGrid)
    {
        sRenderDebugGrid();
    }

    window.display();
}

/**
 * Does the specified action.
 * 
 * Example:
 * An action of type START and name TOGGLE_GRID will toggle the debug grid.
 */
void Scene_Play::sDoAction(const Action & action)
{
    // Toggle Actions
    if (action.type() == "START")
    {
        if (action.name() == "TOGGLE_GRID")
        {
            m_drawGrid = !m_drawGrid;
            return;
        }
        else if (action.name() == "TOGGLE_BOUNDING_BOXES")
        {
            m_drawCollision = !m_drawCollision;
            return;
        }

        else if (action.name() == "TOGGLE_TEXTURES")
        {
            m_drawTextures = !m_drawTextures;
            return;
        }
    }

    bool newState;
    if (action.type() == "START") // START
    {
        newState = true;
    }
    else // END
    {
        newState = false;
    }

    if (action.name() == "UP")
    {
        m_player->getComponent<CInput>().up = newState;
    }
    else if (action.name() == "DOWN")
    {
        m_player->getComponent<CInput>().down = newState;
    }
    else if (action.name() == "LEFT")
    {
        m_player->getComponent<CInput>().left = newState;
    }
    else if (action.name() == "RIGHT")
    {
        m_player->getComponent<CInput>().right = newState;
    }
    else if (action.name() == "RUN")
    {
        m_player->getComponent<CInput>().B = newState;
    }
    else if (action.name() == "JUMP")
    {
        m_player->getComponent<CInput>().A = newState;
    }
}

/**
 * NOT YET IMPLEMENTED.
 * 
 * Purpose is unkown.
 */
void Scene_Play::sDebug()
{   
}

/**
 * NOT YET IMPLEMENTED.
 * 
 * Will be called when this scene is over.
 * It will change the scene from Scene_Play to Scene_Menu.
 */
void Scene_Play::onEnd() 
{
}