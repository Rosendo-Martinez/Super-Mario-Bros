#pragma once

#include "Scene.h"
#include "Action.h"
#include "Entity.h"
#include "Vec2.h"
#include <memory>
#include <string>


class Scene_Play : public Scene
{
    struct GROUNDED_HORIZONTAL_KINEMATICS {
        const double MIN_WALK_SPEED           = 0.296875;
        const double MAX_WALK_SPEED           = 6.25;
        const double MAX_RUN_SPEED            = 10.25;
        const double SKID_TURNAROUND_SPEED    = 2.25;
        const double WALK_ACC                 = 0.1484375;
        const double RUN_ACC                  = 0.22265625;
        const double RELEASE_DEC              = 0.203125;
        const double SKID_DEC                 = 0.40625;
    };

    struct AIRBORNE_HORIZONTAL_KINEMATICS {
        const double CURRENT_SPEED_THRESHOLD_FOR_ACC = 6.25; // CST
        const double INITIAL_SPEED_THRESHOLD_FOR_ACC = 7.25; // IST

        const double BELOW_CST_ACC = 0.1484375; // Acceleration when current speed < CST
        const double ABOVE_CST_ACC = 0.22265625; // Acceleration when current speed >= CST

        const double ABOVE_CST_DEC = 0.22265625;  // Deceleration when current speed >= CST
        const double ABOVE_IST_DEC = 0.203125; // Deceleration when IST <= current speed < CST
        const double BELOW_IST_DEC = 0.1484375; // Deceleration when current speed < IST

        const double INITIAL_SPEED_THRESHOLD_FOR_VEL = 6.25;
        const double BELLOW_ISP_SPEED_LIMIT_VEL = 6.25; // Speed limit if mario started airborne at x speed bellow CST
        const double ABOVE_IST_SPEED_LIMIT_VEL = 10.25; // Speed limit if mario started airborne at x speed above CST
    };

    struct AIRBORNE_VERTICAL_KINEMATICS {
        const double SMALL_SPEED_THRESHOLD = 4; // SST
        const double MEDIUM_SPEED_THRESHOLD = 9.2490234375; // MST
        const double LARGE_SPEED_THRESHOLD = 9.25; // LST

        // < SST
        const double INITIAL_VELOCITY_S = 16;
        const double REDUCED_GRAVITY_S = 0.5;
        const double GRAVITY_S = 1.75;

        // >= SST AND <= MST
        const double INITIAL_VELOCITY_M = 16;
        const double REDUCED_GRAVITY_M = 0.46875;
        const double GRAVITY_M = 1.5;

        // >= LST
        const double INITIAL_VELOCITY_L = 20;
        const double REDUCED_GRAVITY_L = 0.625;
        const double GRAVITY_L = 2.25;

        const double MAX_DOWNWARD_SPEED = 18; // Max speed mario can move downward
        const double RESET_DOWNWARD_SPEED = 16; // Speed to reset to when mario exceeds max downward speed

        const double GOOMBA_STOMP_VELOCITY = 17.25;
    };

    struct PlayerConfig {
        float X, Y, CX, CY, SPEED, MAX_SPEED, JUMP, GRAVITY;
        std::string WEAPON;
    };

private:
    std::shared_ptr<Entity> m_player;
    // Path to level specification file
    std::string m_levelPath;
    PlayerConfig m_playerConfig;
    GROUNDED_HORIZONTAL_KINEMATICS m_groundedHK;
    AIRBORNE_HORIZONTAL_KINEMATICS m_airborneHK;
    AIRBORNE_VERTICAL_KINEMATICS m_jumpVK;
    bool m_drawTextures = true;
    bool m_drawCollision = false;
    bool m_drawGrid = false;
    const Vec2 m_gridSize = { 64, 64 };
    sf::Text m_gridText;
    Vec2 m_cameraPosition; // relative to game map

    void init();
    Vec2 gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity);
    void loadLevel();
    
    void spawnPlayer();
    void spawnBullet(std::shared_ptr<Entity> entity);
    void sPlayerAirBorneMovement();
    void sPlayerGroundedMovement();
    void sPlayerState();

public:
    Scene_Play(GameEngine * gameEngine, const std::string & levelPath);

    void update(); // update EM, and cal systems

    void sAnimation();
    void sMovement();
    void sEnemy();
    void sCollision();
    void sRender();
    void sDoAction(const Action & action); // do the action
    void sDebug();

    void onEnd(); // before scene ends change to menu scene

};