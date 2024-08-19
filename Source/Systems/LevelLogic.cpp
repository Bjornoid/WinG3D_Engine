#include <random>
#include "LevelLogic.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "../Components/Visuals.h"
#include "../Entities/Prefabs.h"
#include "../Utils/Macros.h"
#include "../Components/Gameplay.h"

using namespace Wing3D; // Example Space Game

// Connects logic to traverse any players and allow a controller to manipulate them
bool Wing3D::LevelLogic::Init(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig,
	GW::AUDIO::GAudio _audioEngine)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;
	audioEngine = _audioEngine;
	// create an asynchronus version of the world
	gameAsync = game->async_stage(); // just used for adding stuff, don't try to read data
	gameLock.Create();
	// Pull enemy Y start location from config file
	std::shared_ptr<const GameConfig> readCfg = _gameConfig.lock();
	

	// create a system the runs at the end of the frame only once to merge async changes
	struct LevelSystem {}; // local definition so we control iteration counts
	game->entity("Level System").add<LevelSystem>();
	// only happens once per frame at the very start of the frame
	game->system<LevelSystem>().kind(flecs::OnLoad) // first defined phase
		.each([this](flecs::entity e, LevelSystem& s)
			{
				// merge any waiting changes from the last frame that happened on other threads
				gameLock.LockSyncWrite();
				gameAsync.merge();
				gameLock.UnlockSyncWrite();
			});

	game->system<LevelSystem>().each([this, readCfg](flecs::entity ent, LevelSystem& s)
		{
			flecs::entity playerEnt;
			if (RetreivePrefab("Player", playerEnt))
			{
				gameLock.LockSyncWrite();
				gameAsync.entity().is_a(playerEnt)
					.set<Position>({ 0, 0, 0 })
					.set<Model>({ "Pig" })
					.set<Orientation>({ 0, 0, 0 })
					.set<Camera>({ { 0, 15, -20 }, { -18, 0, 0 } })
					.set<Material>({ 1, 1, 1 })
					.set<ControllerID>({ 0 })
					.add<Player>();

				gameLock.UnlockSyncWrite();
			}
		}
	);

	// Load and play level one's music
	//std::string musicPath = (*readCfg).at("Level").at("music").as<std::string>();
	//currentTrack.Create(musicPath.c_str(), audioEngine, 0.025f);
	//currentTrack.Play(true);

	return true;
}

// Free any resources used to run this system
bool Wing3D::LevelLogic::Shutdown()
{
	timedEvents = nullptr; // stop adding enemies
	gameAsync.merge(); // get rid of any remaining commands
	game->entity("Level System").destruct();
	// invalidate the shared pointers
	game.reset();
	gameConfig.reset();
	return true;
}

// Toggle if a system's Logic is actively running
bool Wing3D::LevelLogic::Activate(bool runSystem)
{
	if (runSystem) {
		game->entity("Level System").enable();
	}
	else {
		game->entity("Level System").disable();
	}
	return false;
}

// **** SAMPLE OF MULTI_THREADED USE ****
//flecs::world world; // main world
//flecs::world async_stage = world.async_stage();
//
//// From thread
//lock(async_stage_lock);
//flecs::entity e = async_stage.entity().child_of(parent)...
//unlock(async_stage_lock);
//
//// From main thread, periodic
//lock(async_stage_lock);
//async_stage.merge(); // merge all commands to main world
//unlock(async_stage_lock);