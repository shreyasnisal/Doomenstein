#include "Game/App.hpp"

#include "Game/GameCommon.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/VirtualReality/OpenXR.hpp"


App* g_app = nullptr;
AudioSystem* g_audio = nullptr;
RandomNumberGenerator* g_RNG = nullptr;
Renderer* g_renderer = nullptr;
Window* g_window = nullptr;
BitmapFont* g_squirrelFont = nullptr;
ModelLoader* g_modelLoader = nullptr;

bool App::HandleQuitRequested(EventArgs& args)
{
	UNUSED(args);
	g_app->HandleQuitRequested();
	return true;
}

bool App::ShowControls(EventArgs& args)
{
	UNUSED(args);

	// Add controls to DevConsole
	g_console->AddLine(Rgba8::STEEL_BLUE, "App-level Controls", false);
	g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Opens the console", "`"), false);
	g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Toggle music mute", "F2"), false);

	g_console->AddLine(Rgba8::STEEL_BLUE, "Attract Screen Controls", false);
	g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Start Game", "Space"), false);
	g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Exits the application", "Escape"), false);
	//g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Add player for controller and go to lobby", "Xbox controller START"), false);

	//g_console->AddLine(Rgba8::STEEL_BLUE, Stringf("Lobby Controls"), false);
	//g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Add player for control type/Start game", "Space/Xbox controller START"), false);
	//g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Go back to attract screen (only works for input modality for which player has been added)", "Escape/Xbox controller BACK"), false);

	g_console->AddLine(Rgba8::STEEL_BLUE, "Game Controls", false);
	g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Returns to attract screen", "Escape"), false);
	g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Pauses/Resumes the game", "P"), false);
	g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Runs the game at timeScale = 0.1f", "T (hold)"), false);
	g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Runs a single frame (and pauses if not already paused)", "O (hold)"), false);
	g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Aim (Yaw/Pitch)", "Mouse Move"), false);
	g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Move", "W/S"), false);
	g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Strafe", "A/D"), false);
	g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Run", "Shift (Hold)"), false);
	g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Fire equipped weapon", "LMB"), false);
	g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Equip weapons", "1/2/3"), false);
	g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Equip previous weapon", "Q"), false);
	g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Equip next weapon", "E"), false);
	g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Start wave (when not in combat mode)", "R"), false);
	g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Elevate (Free-fly only)", "Z/C"), false);
	//g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Debug possess next actor (Single player only)", "N"), false);
	g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Toggle Free-Fly mode", "F"), false);

	return true;
}

XREye App::GetCurrentEye() const
{
	return m_currentEye;
}

Camera const App::GetCurrentCamera() const
{
	if (m_currentEye == XREye::LEFT)
	{
		return m_leftEyeCamera;
	}

	if (m_currentEye == XREye::RIGHT)
	{
		return m_rightEyeCamera;
	}

	return m_worldCamera;
}

void App::InitializeCameras()
{
	m_worldCamera.SetRenderBasis(Vec3::SKYWARD, Vec3::WEST, Vec3::NORTH);
	m_worldCamera.SetPerspectiveView(g_window->GetAspect(), 60.f, 0.1f, 1000.f);
	m_worldCamera.SetTransform(Vec3::ZERO, EulerAngles::ZERO);

	m_screenCamera.SetOrthoView(Vec2::ZERO, Vec2(g_screenSizeY * g_window->GetAspect(), g_screenSizeY));
	m_screenCamera.SetViewport(Vec2::ZERO, Vec2(g_screenSizeY * g_window->GetAspect(), g_screenSizeY));
	m_screenRTVTexture = g_renderer->CreateRenderTargetTexture("ScreenTexture", IntVec2(int(g_screenSizeY * g_window->GetAspect()), int(g_screenSizeY)));

	m_leftEyeCamera.SetRenderBasis(Vec3::GROUNDWARD, Vec3::WEST, Vec3::NORTH);
	m_rightEyeCamera.SetRenderBasis(Vec3::GROUNDWARD, Vec3::WEST, Vec3::NORTH);
}

App::App()
{

}

App::~App()
{
	delete g_renderer;
	g_renderer = nullptr;

	delete g_input;
	g_input = nullptr;

	delete g_audio;
	g_audio = nullptr;

	delete g_RNG;
	g_RNG = nullptr;

	delete m_game;
	m_game = nullptr;
}

void App::Startup()
{
	LoadGameConfigXml();

	EventSystemConfig eventSystemConfig;
	g_eventSystem = new EventSystem(eventSystemConfig);

	InputConfig inputConfig;
	g_input = new InputSystem(inputConfig);

	WindowConfig windowConfig;
	windowConfig.m_inputSystem = g_input;
	windowConfig.m_windowTitle = "(snisal) Doomenstein";
	windowConfig.m_clientAspect = g_gameConfigBlackboard.GetValue("windowAspect", 1.f);
	g_window = new Window(windowConfig);

	RenderConfig renderConfig;
	renderConfig.m_window = g_window;
	g_renderer = new Renderer(renderConfig);

	DevConsoleConfig devConsoleConfig;
	devConsoleConfig.m_renderer = g_renderer;
	devConsoleConfig.m_consoleFontFilePathWithNoExtension = "Data/Fonts/SquirrelFixedFont";
	m_devConsoleCamera.SetOrthoView(Vec2::ZERO, Vec2(g_gameConfigBlackboard.GetValue("windowAspect", 1.f), 1.f));
	devConsoleConfig.m_camera = m_devConsoleCamera;
	g_console = new DevConsole(devConsoleConfig);

	AudioConfig audioConfig;
	g_audio = new AudioSystem(audioConfig);

	DebugRenderConfig debugRenderConfig;
	debugRenderConfig.m_renderer = g_renderer;
	debugRenderConfig.m_bitmapFontFilePathWithNoExtension = "Data/Fonts/SquirrelFixedFont";

	ModelLoaderConfig modelLoaderConfig;
	modelLoaderConfig.m_renderer = g_renderer;
	g_modelLoader = new ModelLoader(modelLoaderConfig);

	OpenXRConfig openXRConfig;
	openXRConfig.m_renderer = g_renderer;
	g_openXR = new OpenXR(openXRConfig);

	g_eventSystem->Startup();
	g_console->Startup();
	g_input->Startup();
	g_window->Startup();
	g_renderer->Startup();
	g_audio->Startup();
	DebugRenderSystemStartup(debugRenderConfig);
	g_modelLoader->Startup();
	g_openXR->Startup();

	InitializeCameras();

	m_game = new Game();

	SubscribeEventCallbackFunction("Quit", HandleQuitRequested, "Exits the application");
	SubscribeEventCallbackFunction("Controls", ShowControls, "Shows game controls");

	EventArgs emptyArgs;
	ShowControls(emptyArgs);
}

void App::LoadGameConfigXml()
{
	g_gameConfigBlackboard = NamedStrings();
	XmlDocument gameConfigXmlFile("Data/GameConfig.xml");
	XmlResult loadResult = gameConfigXmlFile.LoadFile("Data/GameConfig.xml");
	if (loadResult == XmlResult::XML_SUCCESS)
	{
		DebuggerPrintf("GameConfig.xml loaded successfully\n");
		XmlElement* gameConfigRootElement = gameConfigXmlFile.RootElement();
		g_gameConfigBlackboard.PopulateFromXmlElementAttributes(*gameConfigRootElement);

		g_screenSizeY = g_gameConfigBlackboard.GetValue("screenSizeY", g_screenSizeY);
		g_screenSizeX = g_screenSizeY * g_gameConfigBlackboard.GetValue("windowAspect", 1.f);
	}
	else
	{
		ERROR_RECOVERABLE("Could not load Data/GameConfig.xml");
	}
}

void App::Run()
{
	while (!IsQuitting())
	{
		RunFrame();
	}
}

void App::RunFrame()
{
	BeginFrame();

	Update();

	m_currentEye = XREye::NONE;
	g_renderer->BeginRenderForEye(XREye::NONE);

	RenderCustomScreens();

	g_renderer->BeginRenderEvent("Screen to Texture");
	RenderScreen();
	g_renderer->EndRenderEvent("Screen to Texture");

	g_renderer->ClearScreen(Rgba8::BLACK);
	g_renderer->BeginRenderEvent("Desktop Single View");
	Render();
	g_renderer->EndRenderEvent("Desktop Single View");

	if (g_openXR->IsInitialized())
	{
		m_currentEye = XREye::LEFT;
		g_renderer->BeginRenderForEye(XREye::LEFT);
		g_renderer->BeginRenderEvent("HMD Left Eye");
		g_renderer->ClearScreen(Rgba8::BLACK);
		Render();
		g_renderer->EndRenderEvent("HMD Left Eye");

		m_currentEye = XREye::RIGHT;
		g_renderer->BeginRenderForEye(XREye::RIGHT);
		g_renderer->BeginRenderEvent("HMD Right Eye");
		g_renderer->ClearScreen(Rgba8::BLACK);
		Render();
		g_renderer->EndRenderEvent("HMD Right Eye");
	}

	EndFrame();

}

bool App::HandleQuitRequested()
{
	m_isQuitting = true;

	return true;
}

void App::BeginFrame()
{
	Clock::TickSystemClock();

	m_frameRate = 1.f / Clock::GetSystemClock().GetDeltaSeconds();

	g_eventSystem->BeginFrame();
	g_console->BeginFrame();
	g_input->BeginFrame();
	g_window->BeginFrame();
	g_renderer->BeginFrame();
	g_audio->BeginFrame();
	DebugRenderBeginFrame();
	g_modelLoader->BeginFrame();
	g_openXR->BeginFrame();
}

void App::Update()
{
	m_game->Update();

	if (g_input->WasKeyJustPressed(KEYCODE_TILDE))
	{
		g_console->ToggleMode(DevConsoleMode::OPENFULL);
	}
	if (g_input->WasKeyJustPressed(KEYCODE_F2))
	{
		m_game->m_isMusicMuted = !m_game->m_isMusicMuted;
		if (g_audio->IsPlaying(m_game->m_gameMusicPlayback))
		{
			g_audio->SetSoundPlaybackVolume(m_game->m_gameMusicPlayback, m_game->m_isMusicMuted ? 0.f : m_game->m_musicVolume);
		}
		if (g_audio->IsPlaying(m_game->m_attractMusicPlayback))
		{
			g_audio->SetSoundPlaybackVolume(m_game->m_attractMusicPlayback, m_game->m_isMusicMuted ? 0.f : m_game->m_musicVolume);
		}
	}

	// Set cursor modes
	if (g_console->GetMode() != DevConsoleMode::HIDDEN || !g_window->HasFocus() || m_game->m_gameState == GameState::ATTRACT)
	{
		g_input->SetCursorMode(false, false);		
	}
	else
	{
		g_input->SetCursorMode(true, true);
	}

	DebugAddScreenText(Stringf("[System Clock]\t\tTime: %.2f, Frames per Seconds: %.2f, Scale: %.2f", Clock::GetSystemClock().GetTotalSeconds(), m_frameRate, Clock::GetSystemClock().GetTimeScale()), Vec2(g_gameConfigBlackboard.GetValue("screenSizeX", g_screenSizeX) - 16.f, g_gameConfigBlackboard.GetValue("screenSizeY", g_screenSizeY) - 16.f), 16.f, Vec2(1.f, 1.f), 0.f);
}

void App::Render() const
{
	if (m_currentEye == XREye::NONE)
	{
		g_renderer->BeginCamera(m_worldCamera);
	}
	else if (m_currentEye == XREye::LEFT)
	{
		g_renderer->BeginCamera(m_leftEyeCamera);
	}
	else if (m_currentEye == XREye::RIGHT)
	{
		g_renderer->BeginCamera(m_rightEyeCamera);
	}

	// Render game
	m_game->Render();

	if (m_currentEye == XREye::NONE)
	{
		g_renderer->EndCamera(m_worldCamera);
		DebugRenderWorld(m_worldCamera);
	}
	else if (m_currentEye == XREye::LEFT)
	{
		g_renderer->EndCamera(m_leftEyeCamera);
		DebugRenderWorld(m_leftEyeCamera);
	}
	else if (m_currentEye == XREye::RIGHT)
	{
		g_renderer->EndCamera(m_rightEyeCamera);
		DebugRenderWorld(m_rightEyeCamera);
	}
}

void App::RenderCustomScreens() const
{
	m_game->RenderCustomScreens();
}

void App::RenderScreen() const
{
	g_renderer->BindTexture(nullptr);

	g_renderer->SetRTV(m_screenRTVTexture);
	g_renderer->ClearRTV(Rgba8::TRANSPARENT_BLACK, m_screenRTVTexture);

	g_renderer->BeginCamera(m_screenCamera);
	m_game->RenderScreen();
	g_renderer->EndCamera(m_screenCamera);

	DebugRenderScreen(m_screenCamera);

	// Render Dev Console
	g_renderer->SetBlendMode(BlendMode::ALPHA);
	g_renderer->SetDepthMode(DepthMode::DISABLED);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_NONE);
	g_renderer->SetModelConstants();
	g_renderer->BindShader(nullptr);
	g_console->Render(AABB2(Vec2(0.f, 0.f), Vec2(g_window->GetAspect(), 1.f)));


	g_renderer->SetRTV();
}

void App::EndFrame()
{
	g_openXR->EndFrame();
	g_modelLoader->EndFrame();
	DebugRenderEndFrame();
	g_audio->EndFrame();
	g_renderer->EndFrame();
	g_window->EndFrame();
	g_input->EndFrame();
	g_console->EndFrame();
	g_eventSystem->EndFrame();
}

void App::Shutdown()
{
	g_openXR->Shutdown();
	g_modelLoader->Shutdown();
	DebugRenderSystemShutdown();
	g_audio->Shutdown();
	g_renderer->Shutdown();
	g_input->Shutdown();
	g_window->Shutdown();
	g_console->Shutdown();
	g_eventSystem->EndFrame();
}

