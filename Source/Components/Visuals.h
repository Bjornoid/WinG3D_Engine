// define all ECS components related to drawing
#ifndef VISUALS_H
#define VISUALS_H

// snake game (avoid name collisions)
namespace Wing3D
{
	struct Color { GW::MATH2D::GVECTOR3F value; };

	struct Material {
		Wing3D::Color diffuse = { 1, 1, 1 };
	};
};

#endif