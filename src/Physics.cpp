#include "Physics.h"
#include "Components.h"
#include <cmath>

Vec2 Physics::GetOverLap(Vec2 aPos, Vec2 bPos, Vec2 aHalfSize, Vec2 bHalfSize)
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
    
    return GetOverLap(aCT.pos, bCT.pos, aCB.halfSize, bCB.halfSize);
}

Vec2 Physics::GetPreviousOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
    CTransform aCT = a->getComponent<CTransform>();
    CTransform bCT = b->getComponent<CTransform>();
    CBoundingBox aCB = a->getComponent<CBoundingBox>();
    CBoundingBox bCB = b->getComponent<CBoundingBox>();

    return GetOverLap(aCT.prevPos, bCT.prevPos, aCB.halfSize, bCB.halfSize);
}

bool Physics::IsCollision(const Vec2 & overlap)
{
    return (overlap.x > 0) && (overlap.y > 0);
}

// Direction that player came from relative to block.
CollisionDirection Physics::GetCollisionDirection(Vec2 prevOverlap, Vec2 prevPosPlayer, Vec2 prevPosBlock)
{
    // Horizontal direction
    if (prevOverlap.y > 0) {
        // came from left
        if (prevPosPlayer.x < prevPosBlock.x)
        {
            return CollisionDirection::LEFT;
        }
        // came from right
        else
        {
            return CollisionDirection::RIGHT;
        }
    }
    // Vertical direction
    else if (prevOverlap.x > 0)
    {
        // came from top
        if (prevPosPlayer.y < prevPosBlock.y)
        {
            return CollisionDirection::TOP;
        }
        // came from bottom
        else
        {
            return CollisionDirection::BOTTOM;
        }
    }
    // Diagonal direction
    else
    {
        // came from top
        if (prevPosPlayer.y < prevPosBlock.y)
        {
            // came from left
            if (prevPosPlayer.x < prevPosBlock.x)
            {
                return CollisionDirection::DIAGONAL_TOP_LEFT;
            }
            // came from right
            else
            {
                return CollisionDirection::DIAGONAL_TOP_RIGHT;
            }
        }
        // came from bottom
        else
        {
            // came from left
            if (prevPosPlayer.x < prevPosBlock.x)
            {
                return CollisionDirection::DIAGONAL_BOTTOM_LEFT;
            }
            // came from right
            else
            {
                return CollisionDirection::DIAGONAL_BOTTOM_RIGHT;
            }   
        }
    }
}