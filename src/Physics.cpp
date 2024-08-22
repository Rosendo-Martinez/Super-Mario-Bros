#include "Physics.h"
#include "Components.h"
#include <cmath>

Vec2 getOverLap(Vec2 aPos, Vec2 bPos, Vec2 aHalfSize, Vec2 bHalfSize)
{
    Vec2 delta(std::abs(aPos.x - bPos.x), std::abs(aPos.y - bPos.y));
    float ox = aHalfSize.x + bHalfSize.x - delta.x;
    float oy = aHalfSize.y + bHalfSize.y - delta.y;
    
    return Vec2(ox,oy);
}

Vec2 Physics::GetOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
    CTransform aCT = a->getComponent<CTransform>();
    CTransform bCT = b->getComponent<CTransform>();
    CBoundingBox aCB = a->getComponent<CBoundingBox>();
    CBoundingBox bCB = b->getComponent<CBoundingBox>();
    
    return getOverLap(aCT.pos, bCT.pos, aCB.halfSize, bCB.halfSize);
}

Vec2 Physics::GetPreviousOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
    CTransform aCT = a->getComponent<CTransform>();
    CTransform bCT = b->getComponent<CTransform>();
    CBoundingBox aCB = a->getComponent<CBoundingBox>();
    CBoundingBox bCB = b->getComponent<CBoundingBox>();

    return getOverLap(aCT.prevPos, bCT.prevPos, aCB.halfSize, bCB.halfSize);
}