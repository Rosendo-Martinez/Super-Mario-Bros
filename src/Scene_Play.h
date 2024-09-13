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
        const double MIN_WALK_SPEED           = 0.07421875 * 4;
        const double MAX_WALK_SPEED           = 1.5625 * 4;
        const double SKID_TURNAROUND_SPEED    = 0.5625 * 4;
        const double WALK_ACC                 = 0.037109375 * 4;
        const double RUN_ACC                  = 0.0556640625 * 4;
        const double RELEASE_DEC              = 0.05078125 * 4;
        const double SKID_DEC                 = 0.1015625 * 4;
    };

    struct AIRBORNE_HORIZONTAL_KINEMATICS {
        const double CURRENT_SPEED_THRESHOLD = 1.5625 * 4; // CST
        const double INITIAL_SPEED_THRESHOLD = 1.8125 * 4; // IST

        const double BELOW_CST_ACC = 0.037109375 * 4; // Acceleration when current speed < CST
        const double ABOVE_CST_ACC = 0.0556640625 * 4; // Acceleration when current speed >= CST

        const double ABOVE_CST_DEC = 0.0556640625 * 4;  // Deceleration when current speed >= CST
        const double ABOVE_IST_DEC = 0.05078125 * 4; // Deceleration when IST <= current speed < CST
        const double BELOW_IST_DEC = 0.037109375 * 4; // Deceleration when current speed < IST
    };

    struct AIRBORNE_VERTICAL_KINEMATICS {
        const double SMALL_SPEED_THRESHOLD = 1 * 4; // SST
        const double MEDIUM_SPEED_THRESHOLD = 2.312255859375 * 4; // MST
        const double LARGE_SPEED_THRESHOLD = 2.3125 * 4; // LST

        // < SST
        const double INITIAL_VELOCITY_S = 4 * 4;
        const double REDUCED_GRAVITY_S = 0.125 * 4;
        const double GRAVITY_S = 0.4375 * 4;

        // >= SST AND <= MST
        const double INITIAL_VELOCITY_M = 4 * 4;
        const double REDUCED_GRAVITY_M = 0.1171875 * 4;
        const double GRAVITY_M = 0.375 * 4;

        // >= LST
        const double INITIAL_VELOCITY_L = 5 * 4;
        const double REDUCED_GRAVITY_L = 0.15625 * 4;
        const double GRAVITY_L = 0.5625 * 4;
    };

    struct PlayerConfig {
        float X, Y, CX, CY, SPEED, MAX_SPEED, JUMP, GRAVITY;
        std::string WEAPON;
    };

private:
    std::shared_ptr<Entity> m_player;
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

    void init(const std::string & levelPath); // register actions, font/text, loadlevel(path)
    Vec2 gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity);
    void loadLevel(const std::string & filename); // load/reset/reload level
    
    void spawnPlayer();
    void spawnBullet(std::shared_ptr<Entity> entity);

public:
    Scene_Play(GameEngine * gameEngine, const std::string & levelPath);

    void update(); // update EM, and cal systems

    void sAnimation();
    void sMovement();
    void sEnemySpawn();
    void sCollision();
    void sRender();
    void sDoAction(const Action & action); // do the action
    void sDebug();

    void onEnd(); // before scene ends change to menu scene

};