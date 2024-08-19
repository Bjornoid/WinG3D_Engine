// define all ECS components related to identification
#ifndef IDENTIFICATION_H
#define IDENTIFICATION_H

// snake game (avoid name collisions)
namespace Wing3D
{
	struct Player {};
	struct ControllerID {
		unsigned index = 0;
	};
	struct Model { 
		const char* name; 
	};
	struct Camera {
		GW::MATH2D::GVECTOR3F Offset;
		GW::MATH2D::GVECTOR3F Orientation;
	};
};

#endif