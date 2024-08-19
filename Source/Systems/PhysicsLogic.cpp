#include "PhysicsLogic.h"
#include "../Components/Physics.h"

bool Wing3D::PhysicsLogic::Init(	std::shared_ptr<flecs::world> _game, 
								std::weak_ptr<const GameConfig> _gameConfig)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;
	// **** COLLISIONS ****
	// due to wanting to loop through all collidables at once, we do this in two steps:
	// 1. A System will gather all collidables into a shared std::vector
	// 2. A second system will run after, testing/resolving all collidables against each other
	queryCache = game->query<Collidable, Position, Orientation>();
	// only happens once per frame at the very start of the frame
	struct CollisionSystem {}; // local definition so we control iteration count (singular)
	game->entity("Detect-Collisions").add<CollisionSystem>();
	game->system<CollisionSystem>()
		.each([this](CollisionSystem& s) {
		
		// collect any and all collidable objects
		queryCache.each([this](flecs::entity e, Collidable& c, Position& p, Orientation& o) {
			SHAPE polygon;


			// add to vector
			testCache.push_back(polygon);
		});
		// loop through the testCahe resolving all collisions
		for (int i = 0; i < testCache.size(); ++i) {
			// the inner loop starts at the entity after you so you don't double check collisions
			for (int j = i + 1; j < testCache.size(); ++j) {

				
			}
		}
		// wipe the test cache for the next frame (keeps capacity intact)
		testCache.clear();
	});
	return true;
}

bool Wing3D::PhysicsLogic::Activate(bool runSystem)
{
	if (runSystem) {
		game->entity("Acceleration System").enable();
		game->entity("Translation System").enable();
		game->entity("Cleanup System").enable();
	}
	else {
		game->entity("Acceleration System").disable();
		game->entity("Translation System").disable();
		game->entity("Cleanup System").disable();
	}
	return true;
}

bool Wing3D::PhysicsLogic::Shutdown()
{
	queryCache.destruct(); // fixes crash on shutdown
	game->entity("Acceleration System").destruct();
	game->entity("Translation System").destruct();
	game->entity("Cleanup System").destruct();
	return true;
}
