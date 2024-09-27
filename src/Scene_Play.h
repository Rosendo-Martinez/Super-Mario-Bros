#pragma once

#include "Scene.h"
#include "Action.h"
#include "Entity.h"
#include "Vec2.h"
#include <memory>
#include <string>

class Scene_Play : public Scene {
private:
    std::shared_ptr<Entity> m_player;
    
    // Path to level specification file
    std::string m_levelPath;
    
    // Rendering flags
    bool m_drawTextures = true;
    bool m_drawCollision = false;
    bool m_drawGrid = false;
    
    // Grid and camera settings
    const Vec2 m_gridCellSize = { 64.f, 64.f };
    sf::Text m_gridText;
    Vec2 m_cameraPosition = { 0.f, 0.f }; // Top left corner of the camera

    // Initialization functions
    void init();
    void loadLevel();
    void createStaticEntity(const std::string& type, const std::string& animation, float gx, float gy);
    void createEnemyEntity(const std::string& type, float gx, float gy, float activationDistance);
    void spawnPlayer();
    void spawnBullet(std::shared_ptr<Entity> entity);

    // Utility functions
    Vec2 gridToCartesianRepresentation(float gridX, float gridY, std::shared_ptr<Entity> entity);
    Vec2 gridToCartesianRepresentation(Vec2 gridPos, Vec2 entitySize);
    
    void reloadLevel();

    // Player-related systems
    void sPlayerAirBorneMovement();
    void sPlayerGroundedMovement();
    void sPlayerState();
    void sPlayerAnimation();
    void sPlayerCollision();
    
    // Enemy-related systems
    void sEnemyCollision();

    // Rendering systems
    void sRenderEntities(EntityVec& entities);
    void sRenderBoundingBoxes();
    void sRenderDebugGrid();

    // General systems
    void sAnimation();
    void sMovement();
    void sEnemyState();
    void sCollision();
    void sRender();
    void sDebug();

public:
    Scene_Play(GameEngine* gameEngine, const std::string& levelPath);

    void update();
    void sDoAction(const Action& action);
    void onEnd();
};
