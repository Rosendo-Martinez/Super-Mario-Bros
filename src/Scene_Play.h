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
    std::string m_levelPath;
    PlayerConfig m_playerConfig;
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