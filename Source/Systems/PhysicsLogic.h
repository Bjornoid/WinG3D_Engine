// The level system is responsible for transitioning the various levels in the game
#ifndef PHYSICSLOGIC_H
#define PHYSICSLOGIC_H

// Contains our global game settings
#include "../GameConfig.h"
#include "../Components/Physics.h"

// snake game (avoid name collisions)
namespace Wing3D
{
	class PhysicsLogic
	{
		// shared connection to the main ECS engine
		std::shared_ptr<flecs::world> game;
		// non-ownership handle to configuration settings
		std::weak_ptr<const GameConfig> gameConfig;
		// used to cache collision queries
		flecs::query<Collidable, Position, Orientation> queryCache;
		// defines what to be tested
		struct SHAPE {

		};

		// vector used to save/cache all active collidables
		std::vector<SHAPE> testCache;
	public:
		// attach the required logic to the ECS 
		bool Init(	std::shared_ptr<flecs::world> _game,
					std::weak_ptr<const GameConfig> _gameConfig);
		// control if the system is actively running
		bool Activate(bool runSystem);
		// release any resources allocated by the system
		bool Shutdown();
	};

};

#endif