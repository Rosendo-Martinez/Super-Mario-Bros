#pragma once

#include "Scene.h"
#include "Action.h"
#include "Entity.h"
#include "Vec2.h"
#include <memory>
#include <string>


class Scene_Play : public Scene
{
    struct PlayerConfig {
        float X, Y, CX, CY, SPEED, MAX_SPEED, JUMP, GRAVITY;
        std::string WEAPON;
    };

private:
    std::shared_ptr<Entity> m_player;
    // Path to level specification file
    std::string m_levelPath;
    PlayerConfig m_playerConfig;
    bool m_drawTextures = true;
    bool m_drawCollision = false;
    bool m_drawGrid = false;
    const Vec2 m_gridCellSize = { 64, 64 };
    sf::Text m_gridText;
    // Top left corner of camera
    Vec2 m_cameraPosition {0.f,0.f};

    void init();
    Vec2 gridToCartesianRepresentation(float gridX, float gridY, std::shared_ptr<Entity> entity);
    Vec2 gridToCartesianRepresentation(Vec2 gridPos, Vec2 entitySize);
    void loadLevel();
    void createStaticEntity(std::string type, std::string animation, float gx, float gy);
    void createEnemyEntity(std::string type, float gx, float gy, float activationDistance);
    
    void spawnPlayer();
    void spawnBullet(std::shared_ptr<Entity> entity);
    void sPlayerAirBorneMovement();
    void sPlayerGroundedMovement();
    void sPlayerState();
    void sPlayerAnimation();
    void sPlayerCollision();
    void sEnemyCollision();
    void sRenderEntities(EntityVec & entities);
    void sRenderBoundingBoxes();
    void sRenderDebugGrid();

public:
    Scene_Play(GameEngine * gameEngine, const std::string & levelPath);

    void update(); // update EM, and cal systems

    void sAnimation();
    void sMovement();
    void sEnemyState();
    void sCollision();
    void sRender();
    void sDoAction(const Action & action); // do the action
    void sDebug();

    void onEnd(); // before scene ends change to menu scene

};