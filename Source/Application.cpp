#include "Application.h"


// open some Gateware namespaces for conveinence 
// NEVER do this in a header file!
using namespace GW;
using namespace CORE;
using namespace SYSTEM;
using namespace GRAPHICS;


bool Application::Init() 
{
	eventPusher.Create();


	// load all game settigns
	gameConfig = std::make_shared<GameConfig>(); 
	// create the ECS system
	game = std::make_shared<flecs::world>(); 
	// init all other systems
	if (InitWindow() == false) 
		return false;
	if (InitInput() == false)
		return false;
	if (InitAudio() == false)
		return false;
	if (InitGraphics() == false)
		return false;
	if (InitEntities() == false)
		return false;
	if (InitSystems() == false)
		return false;
	return true;
}

bool Application::Run() 
{
	float r = gameConfig->at("BackgroundColor").at("red").as<float>();
	float g = gameConfig->at("BackgroundColor").at("green").as<float>();
	float b = gameConfig->at("BackgroundColor").at("blue").as<float>();
	float backgroundColor[4] = {r, g, b, 1};

	bool winClosed = false;
	GEventResponder msgs;
	msgs.Create([&](const GW::GEvent& e) {
		GW::SYSTEM::GWindow::Events q;
		if (+e.Read(q) && q == GWindow::Events::RESIZE)
			backgroundColor[0] += 0.01f; // move towards a orange as they resize
		if (+e.Read(q) && q == GWindow::Events::DESTROY)
			winClosed = true;
		});

	DX12RenderingSystem.SetBackgroundColor(backgroundColor);

	window.Register(msgs);
	while (+window.ProcessWindowEvents())
	{
		if (winClosed == true)
			return true;

		if (+d3d.StartFrame())
		{
			if (GameLoop() == false) {
				d3d.EndFrame(false);
				return false;
			}

			
			d3d.EndFrame(false);
		}
		else
			return false;
	}

	return true;
}

bool Application::Shutdown() 
{
	// disconnect systems from global ECS
	if (levelSystem.Shutdown() == false)
		return false;
	if (DX12RenderingSystem.Shutdown() == false)
		return false;
	if (physicsSystem.Shutdown() == false)
		return false;

	return true;
}

bool Application::InitWindow()
{
	// grab settings
	int width = gameConfig->at("Window").at("width").as<int>();
	int height = gameConfig->at("Window").at("height").as<int>();
	int xstart = gameConfig->at("Window").at("xstart").as<int>();
	int ystart = gameConfig->at("Window").at("ystart").as<int>();
	std::string title = gameConfig->at("Window").at("title").as<std::string>();
	// open window
	if (+window.Create(xstart, ystart, width, height, GWindowStyle::WINDOWEDBORDERED) &&
		+window.SetWindowName(title.c_str())) {
		return true;
	}
	return false;
}

bool Application::InitInput()
{
	if (-gamePads.Create())
		return false;
	if (-immediateInput.Create(window))
		return false;
	if (-bufferedInput.Create(window))
		return false;
	return true;
}

bool Application::InitAudio()
{
	if (-audioEngine.Create())
		return false;
	return true;
}

bool Application::InitGraphics()
{
	if (+d3d.Create(window, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT))
		return true;
	
	return false;
}

bool Application::InitEntities()
{
	
	return true;
}

bool Application::InitSystems()
{
	// connect systems to global ECS
	if (levelSystem.Init(game, gameConfig, audioEngine) == false)
		return false;
	if (DX12RenderingSystem.Init(game, gameConfig, d3d, window, immediateInput, gamePads) == false)
		return false;
	if (physicsSystem.Init(game, gameConfig) == false)
		return false;

	return true;
}

bool Application::GameLoop()
{
	// compute delta time and pass to the ECS system
	static auto start = std::chrono::steady_clock::now();
	double elapsed = std::chrono::duration<double>(
		std::chrono::steady_clock::now() - start).count();
	start = std::chrono::steady_clock::now();
	// let the ECS system run

	return game->progress(static_cast<float>(elapsed));
}
