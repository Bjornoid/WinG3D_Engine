#ifndef APPLICATION_H
#define APPLICATION_H

// include events
#include "Events/Playevents.h"
// Contains our global game settings
#include "GameConfig.h"
// Load all entities+prefabs used by the game

// Include all systems used by the game and their associated components
#include "Systems/DirX12RendererLogic.h"
#include "Systems/LevelLogic.h"
#include "Systems/PhysicsLogic.h"

// Allocates and runs all sub-systems essential to operating the game
class Application 
{
	// gateware libs used to access operating system
	GW::SYSTEM::GWindow window; // gateware multi-platform window
	GW::GRAPHICS::GDirectX12Surface d3d; // gateware vulkan API wrapper
	GW::INPUT::GController gamePads; // controller support
	GW::INPUT::GInput immediateInput; // twitch keybaord/mouse
	GW::INPUT::GBufferedInput bufferedInput; // event keyboard/mouse
	GW::AUDIO::GAudio audioEngine; // can create music & sound effects
	// third-party gameplay & utility libraries
	std::shared_ptr<flecs::world> game; // ECS database for gameplay
	std::shared_ptr<GameConfig> gameConfig; // .ini file game settings
	// ECS Entities and Prefabs that need to be loaded

	// specific ECS systems used to run the game
	Wing3D::DirX12RendererLogic DX12RenderingSystem;
	Wing3D::LevelLogic levelSystem;
	Wing3D::PhysicsLogic physicsSystem;

	
	// EventGenerator for Game Events
	GW::CORE::GEventGenerator eventPusher;


public:
	bool Init();
	bool Run();
	bool Shutdown();
private:
	bool InitWindow();
	bool InitInput();
	bool InitAudio();
	bool InitGraphics();
	bool InitEntities();
	bool InitSystems();
	bool GameLoop();
};

#endif 