#pragma once

#include "GameEngine.h"
#include "EntityManager.h"
#include "Action.h"
#include <map>
#include <string>

class Scene 
{
private:
    GameEngine game;
    EntityManager entities;
    int currentFrame;
    std::map<int, std::string> actionMap;
    bool paused;
    bool hasEnded;
public:
    virtual void update() = 0;
    virtual void sDoAction() = 0;
    virtual void sRender() = 0;

    void simulate(int count); // calls derived scene's update() a count number of times
    void doAction(Action action);
    void registerAction(Action action);
};