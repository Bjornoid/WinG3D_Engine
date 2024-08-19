#include "PlayerData.h"
#include "../Components/Identification.h"
#include "../Components/Visuals.h"
#include "../Components/Physics.h"
#include "Prefabs.h"

bool Wing3D::PlayerData::Load(std::shared_ptr<flecs::world> _game, std::weak_ptr<const GameConfig> _gameConfig, GW::AUDIO::GAudio _audioEngine)
{
	// Grab init settings for players
	std::shared_ptr<const GameConfig> readCfg = _gameConfig.lock();

	// Create Player One
	_game->entity("Player")
		.set([&](Position& p, OldPosition& op, Orientation& o, Camera& cam, Material& m, ControllerID& c, Model& mo) {
		c = { 0 };
		p = { 0, 0, 0 };
		m = { 1, 1, 1};
		o = { 0, 0, 0 };
		mo = { "Pig" };
		cam = { {0, 10, -15}, {-18, 0, 0} };
			})
		.add<Collidable>()
		.add<Player>(); // Tag this entity as a Player

	return true;
}

bool Wing3D::PlayerData::Unload(std::shared_ptr<flecs::world> _game)
{
	return true;
}
