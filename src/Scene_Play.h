#pragma once

#include "Scene.h"
#include "Action.h"
#include "Entity.h"
#include <memory>
#include <string>

struct PlayerConfig {};

class Scene_Play : public Scene
{
private:
    std::string m_levelPath;
    std::shared_ptr<Entity> m_player;
    PlayerConfig m_playerConfig;

    void init();
public:
    void update();

    void sAnimation();
    void sMovement();
    void sEnemySpawn();
    void sCollision();
    void sRender();
    void sDoAction(Action action);;
    void sDebug();
};