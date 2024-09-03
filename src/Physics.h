#include "Vec2.h"
#include "Entity.h"

#include <memory>

enum class CollisionDirection 
{
    DIAGONAL_TOP_LEFT,
    DIAGONAL_TOP_RIGHT,
    DIAGONAL_BOTTOM_LEFT,
    DIAGONAL_BOTTOM_RIGHT,
    TOP,
    BOTTOM,
    LEFT,
    RIGHT
};

class Physics
{
public:
    static Vec2 GetOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b);
    static Vec2 GetPreviousOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b);
    static bool IsCollision(const Vec2 & overlap);
    static CollisionDirection GetCollisionDirection(Vec2 prevOverlap, Vec2 prevPosPlayer, Vec2 prevPosBlock); // The direction from which the player came at the block.
};