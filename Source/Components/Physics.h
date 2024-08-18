// define all ECS components related to movement & collision
#ifndef PHYSICS_H
#define PHYSICS_H

// snake game (avoid name collisions)
namespace Wing3D
{
	// ECS component types should be *strongly* typed for proper queries
	// typedef is tempting but it does not help templates/functions resolve type
	struct Position { GW::MATH2D::GVECTOR2F value = GW::MATH2D::GVECTOR2F{ 0, 1 }; };
	struct Velocity { GW::MATH2D::GVECTOR2F value; };
	struct Orientation { GW::MATH2D::GMATRIX2F value; };
	struct OldPosition { GW::MATH2D::GVECTOR2F value; };


	// Individual TAGs
	struct Collidable {};

	// ECS Relationship tags
	struct CollidedWith {};
};

#endif