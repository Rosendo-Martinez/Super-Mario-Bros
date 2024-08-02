#include "Scene.h"

void Scene::onEnd()
{
}

void Scene::setPaused(bool paused)
{
}

Scene::Scene()
{
}

Scene::Scene(GameEngine * gameEngine)
    : m_game(gameEngine)
{
}

void Scene::update()
{
}

void Scene::sDoAction(const Action & action)
{
}

void Scene::sRender()
{
}

void Scene::doAction(const Action & action)
{
}

void Scene::simulate(const size_t frames) // calls derived scene's update() a count number of times
{
}

void Scene::registerAction(int inputKey, const std::string & actionName)
{
}

size_t Scene::width() const
{
}

size_t Scene::height() const
{
}

size_t Scene::currentFrame() const
{
}

bool Scene::hasEnded() const
{
}

const ActionMap & Scene::getActionMap() const
{
}

void Scene::drawLine(const Vec2 & p1, const Vec2 & p2)
{
}