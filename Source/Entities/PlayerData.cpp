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
		.set([&](Position& p, OldPosition& op, Orientation& o, Material& m, ControllerID& c, Model& mo) {
		c = { };
		p = {  };
		m = { };
		o = {  };
		mo = { "Player" };
		GW::MATH2D::GMatrix2D::Scale2F(o.value, GW::MATH2D::GVECTOR2F{  }, o.value);
			})
		.add<Collidable>()
		.add<Player>(); // Tag this entity as a Player

	return true;
}

bool Wing3D::PlayerData::Unload(std::shared_ptr<flecs::world> _game)
{
	return true;
}
