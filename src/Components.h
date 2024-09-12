#pragma once

#include <string>
#include "Vec2.h"

#include "Animation.h"
#include "Assets.h"

class Component
{
public:
    bool has = false;
};

class CTransform : public Component
{
public:
    Vec2  pos        = { 0.0, 0.0 };
    Vec2  prevPos    = { 0.0, 0.0 };
    Vec2  scale      = { 1.0, 1.0 };
    Vec2  velocity   = { 0.0, 0.0 };
    float angle      = 0;
    double acc_x     = 0;
    double acc_y     = 0;

    CTransform() {}
    CTransform(const Vec2 & p)
        : pos(p), prevPos(p) {}
    CTransform(const Vec2 & p, const Vec2 & sp, const Vec2 & sc, float a)
        : pos(p), prevPos(p), velocity(sp), scale(sc), angle(a) {}
};

class CLifeSpan : public Component
{
public:
    int lifespan = 0;
    int frameCreated = 0;
    CLifeSpan() {}
    CLifeSpan(int duration, int frame)
        : lifespan(duration), frameCreated(frame) {}
};

class CInput : public Component
{
public:
    bool up         = false;
    bool down       = false;
    bool left       = false;
    bool right      = false;
    bool A          = false; // Jump & Swim
    bool B          = false; // Accelerate & Shoot
    bool canShoot   = false;
    bool canJump    = false;

    CInput() {}
};

class CBoundingBox : public Component
{
public:
    Vec2 size;
    Vec2 halfSize;
    CBoundingBox() {}
    CBoundingBox(const Vec2 & s)
        : size(s), halfSize(s.x / 2, s.y / 2) {}
};

class CAnimation : public Component
{
public:
    Animation animation;
    bool repeat = false;
    CAnimation() {}
    CAnimation(const Animation & animation, bool r)
        : animation(animation), repeat(r) {}
};

class CGravity : public Component
{
public:
    float gravity = 0;
    CGravity() {}
    CGravity(float g) : gravity(g) {}
};

class CState : public Component
{
public:
    std::string state = "default";
    float initialJumpXSpeed = 0;
    CState() {}
    CState(const std::string & s) : state(s) {}
};
