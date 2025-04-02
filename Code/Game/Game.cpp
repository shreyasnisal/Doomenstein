#include "Game/Game.hpp"

#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Player.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/WeaponDefinition.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/Map.hpp"
#include "Game/Gold/GoldMap.hpp"
#include "Game/Actor.hpp"

#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/Spritesheet.hpp"
#include "Engine/VirtualReality/OpenXR.hpp"

Game::Game()
{
	LoadAssets();
	TileDefinition::InitializeTileDefinitions();
	MapDefinition::InitializeMapDefinitions();
	WeaponDefinition::InitializeWeaponDefinitions();
	ActorDefinition::InitializeActorDefinitions();

	m_player = new Player(this, 0, -1);
}

Game::~Game()
{
}

void Game::LoadAssets()
{
	m_attractScreenBackgroundTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Attract_Background.png");
	g_squirrelFont = g_renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	m_attractMusic = g_audio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("mainMenuMusic", "mainMenuMusic"));
	m_buttonClickSound = g_audio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("buttonClickSound", "buttonClickSound"));
	m_gameMusic = g_audio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("gameMusic", "gameMusic"));
	m_musicVolume = g_gameConfigBlackboard.GetValue("musicVolume", m_musicVolume);

	if (m_gameState == GameState::INTRO)
	{
		m_logoTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Logo.png");
		SoundID logoBackgroundMusic = g_audio->CreateOrGetSound("Data/Audio//Music/LogoMusic.mp3");
		m_introMusicPlayback = g_audio->StartSound(logoBackgroundMusic);
	}
	else
	{
		m_attractMusicPlayback = g_audio->StartSound(m_attractMusic, true, m_musicVolume);
	}
}

void Game::Update()
{
	float deltaSeconds = m_gameClock.GetDeltaSeconds();
	float gameFPS = deltaSeconds == 0.f ? 0.f : 1.f / deltaSeconds;
	DebugAddScreenText(Stringf("[Game Clock]\t\tTime: %.2f, Frames per Seconds: %.2f, Scale: %.2f", m_gameClock.GetTotalSeconds(), gameFPS, m_gameClock.GetTimeScale()), Vec2(g_gameConfigBlackboard.GetValue("screenSizeX", g_screenSizeX) - 16.f, g_gameConfigBlackboard.GetValue("screenSizeY", g_screenSizeY) - 32.f), 16.f, Vec2(1.f, 1.f), 0.f);

	switch (m_gameState)
	{
		//case GameState::INTRO:					UpdateIntroScreen(deltaSeconds);				break;
		case GameState::ATTRACT:				UpdateAttractScreen(deltaSeconds);				break;
		//case GameState::LOBBY:					UpdateLobby(deltaSeconds);						break;
		case GameState::GAME:					UpdateGame(deltaSeconds);						break;
	}


	if (g_openXR && g_openXR->IsInitialized())
	{
		EulerAngles leftEyeOrientation;
		g_openXR->GetTransformForEye_iFwd_jLeft_kUp(XREye::LEFT, m_player->m_leftEyeLocalPosition, leftEyeOrientation);

		EulerAngles rightEyeOrientation;
		g_openXR->GetTransformForEye_iFwd_jLeft_kUp(XREye::RIGHT, m_player->m_rightEyeLocalPosition, rightEyeOrientation);

		m_player->m_hmdOrientation = rightEyeOrientation;
	}

	Mat44 billboardTargetMatrix = Mat44::CreateTranslation3D(m_player->m_position);
	billboardTargetMatrix.Append((m_player->m_orientation + m_player->m_hmdOrientation).GetAsMatrix_iFwd_jLeft_kUp());

	m_screenBillboardMatrix = GetBillboardMatrix(BillboardType::FULL_FACING, billboardTargetMatrix, m_player->m_position + (m_player->m_orientation + m_player->m_hmdOrientation).GetAsMatrix_iFwd_jLeft_kUp().GetIBasis3D() * SCREEN_QUAD_DISTANCE);

}

void Game::UpdateGame(float deltaSeconds)
{
	HandleDeveloperCheats();
	
	if (m_currentMap)
	{
		UpdatePlayers(deltaSeconds);
		if (m_currentMap)
		{
			m_currentMap->Update();
			UpdateCameras();
		}
	}

	m_timeInState += deltaSeconds;
}

void Game::UpdatePlayers(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	m_player->Update();
}

void Game::UpdateCameras()
{
	if (m_currentMap)
	{
		Actor* possessedActor = m_currentMap->GetActorByUID(m_player->m_actorUID);
		if (possessedActor)
		{
			m_player->m_position = possessedActor->GetEyePosition();
			m_player->m_orientation = possessedActor->m_orientation;
		}
	}
	g_app->m_worldCamera.SetTransform(m_player->m_position, m_player->m_orientation);
	
	if (g_openXR && g_openXR->IsInitialized())
	{
		Mat44 playerModelMatrix = Mat44::CreateTranslation3D(m_player->m_position);
		playerModelMatrix.Append(m_player->m_orientation.GetAsMatrix_iFwd_jLeft_kUp());

		constexpr float XR_NEAR = 0.1f;
		constexpr float XR_FAR = 1000.f;
		float lFovLeft, lFovRight, lFovUp, lFovDown;
		float rFovLeft, rFovRight, rFovUp, rFovDown;
		Vec3 leftEyePosition, rightEyePosition;
		EulerAngles leftEyeOrientation, rightEyeOrientation;

		g_openXR->GetFovsForEye(XREye::LEFT, lFovLeft, lFovRight, lFovUp, lFovDown);
		g_app->m_leftEyeCamera.SetXRView(lFovLeft, lFovRight, lFovUp, lFovDown, XR_NEAR, XR_FAR);
		Mat44 leftEyeTransform = playerModelMatrix;
		leftEyeTransform.AppendTranslation3D(m_player->m_leftEyeLocalPosition);
		leftEyeTransform.Append(m_player->m_hmdOrientation.GetAsMatrix_iFwd_jLeft_kUp());
		g_app->m_leftEyeCamera.SetTransform(leftEyeTransform);

		g_openXR->GetFovsForEye(XREye::RIGHT, rFovLeft, rFovRight, rFovUp, rFovDown);
		g_app->m_rightEyeCamera.SetXRView(rFovLeft, rFovRight, rFovUp, rFovDown, XR_NEAR, XR_FAR);
		Mat44 rightEyeTransform = playerModelMatrix;
		rightEyeTransform.AppendTranslation3D(m_player->m_rightEyeLocalPosition);
		rightEyeTransform.Append(m_player->m_hmdOrientation.GetAsMatrix_iFwd_jLeft_kUp());
		g_app->m_rightEyeCamera.SetTransform(rightEyeTransform);
	}


	//g_app->m_screenCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(g_gameConfigBlackboard.GetValue("screenSizeX", g_screenSizeX), g_gameConfigBlackboard.GetValue("screenSizeY", g_screenSizeY)));
}

void Game::HandleDeveloperCheats()
{
	XboxController xboxController = g_input->GetController(0);
	
	if (g_input->IsKeyDown('O'))
	{
		m_gameClock.StepSingleFrame();
	}
	if (g_input->WasKeyJustPressed('T'))
	{
		m_gameClock.SetTimeScale(0.1f);
	}
	if (g_input->WasKeyJustReleased('T'))
	{
		m_gameClock.SetTimeScale(1.f);
	}

	if (g_input->WasKeyJustPressed('P'))
	{
		m_gameClock.TogglePause();
	}
}

void Game::Render() const
{
	switch (m_gameState)
	{
		//case GameState::INTRO:				RenderIntroScreen();					break;
		//case GameState::ATTRACT:			RenderAttractScreen();					break;
		//case GameState::LOBBY:				RenderLobby();							break;
		case GameState::GAME:				RenderGame();							break;
	}
	RenderWorldScreenQuad();
}

void Game::RenderCustomScreens() const
{
	if (m_currentMap)
	{
		m_currentMap->RenderCustomScreens();
	}
}

void Game::RenderScreen() const
{
	switch (m_gameState)
	{
		case GameState::INTRO:				RenderIntroScreen();					break;
		case GameState::ATTRACT:			RenderAttractScreen();					break;
		case GameState::GAME:				RenderGameScreen();						break;
	}
}

void Game::RenderGame() const
{
	if (m_currentMap)
	{
		m_currentMap->Render();
	}

	//g_renderer->BeginCamera(g_app->m_screenCamera);
	//RenderHUD();
	//g_renderer->EndCamera(g_app->m_screenCamera);
}

void Game::RenderGameScreen() const
{
}

void Game::RenderWorldScreenQuad() const
{
	g_renderer->BeginRenderEvent("World Screen Quad");
	{
		// Math for rendering screen quad
		XREye currentEye = g_app->GetCurrentEye();
		float quadHeight = SCREEN_QUAD_DISTANCE * TanDegrees(30.f) * (currentEye == XREye::NONE ? 1.f : 0.5f);
		float quadWidth = quadHeight * g_window->GetAspect();

		// Render screen quad
		std::vector<Vertex_PCU> screenVerts;
		AddVertsForQuad3D(screenVerts, Vec3(0.f, quadWidth, -quadHeight), Vec3(0.f, -quadWidth, -quadHeight), Vec3(0.f, -quadWidth, quadHeight), Vec3(0.f, quadWidth, quadHeight), Rgba8::WHITE, AABB2(Vec2(1.f, 1.f), Vec2(0.f, 0.f)));
		g_renderer->SetBlendMode(BlendMode::ALPHA);
		g_renderer->SetDepthMode(DepthMode::DISABLED);
		g_renderer->SetModelConstants(m_screenBillboardMatrix);
		g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_NONE);
		g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
		g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		g_renderer->BindTexture(g_app->m_screenRTVTexture);
		g_renderer->BindShader(nullptr);
		g_renderer->DrawVertexArray(screenVerts);
	}
	g_renderer->EndRenderEvent("World Screen Quad");
}

void Game::GoToLobby()
{
	DebugRenderClear();
	m_gameState = GameState::LOBBY;
}

void Game::QuitToLobby()
{
	DebugRenderClear();
	delete m_currentMap;
	m_currentMap = nullptr;


	g_audio->StopSound(m_gameMusicPlayback);
	g_audio->StartSound(m_attractMusic, true, m_isMusicMuted ? 0.f : m_musicVolume);

	m_gameState = GameState::LOBBY;
}

void Game::StartGame()
{
	DebugRenderClear();
	m_gameState = GameState::GAME;
	m_timeInState = 0.f;

	//m_currentMap = new Map(this, MapDefinition::s_mapDefs[g_gameConfigBlackboard.GetValue("defaultMap", "")]);
	//for (int playerIndex = 0; playerIndex < (int)m_players.size(); playerIndex++)
	//{
	//	if (m_players[playerIndex])
	//	{
	//		m_currentMap->SpawnPlayer(m_players[playerIndex]->m_playerIndex);
	//	}
	//}

	m_currentMap = new GoldMap(this);

	g_audio->StopSound(m_attractMusicPlayback);
	m_gameMusicPlayback = g_audio->StartSound(m_gameMusic, true, m_isMusicMuted ? 0.f : m_musicVolume);

	g_audio->SetNumListeners(1);
}

void Game::QuitToAttractScreen()
{
	DebugRenderClear();

	if (m_currentMap)
	{
		delete m_currentMap;
		m_currentMap = nullptr;
	}

	if (g_audio->IsPlaying(m_gameMusicPlayback))
	{
		g_audio->StopSound(m_gameMusicPlayback);
		m_attractMusicPlayback = g_audio->StartSound(m_attractMusic, true, m_isMusicMuted ? 0.f : m_musicVolume);
	}

	m_gameState = GameState::ATTRACT;
	m_timeInState = 0.f;
}

void Game::StartGold()
{
	DebugRenderClear();
	m_gameState = GameState::GAME;
	m_timeInState = 0.f;

	m_currentMap = new GoldMap(this);
	m_currentMap->SpawnPlayer(0);

	g_audio->StopSound(m_attractMusicPlayback);
	m_gameMusicPlayback = g_audio->StartSound(m_gameMusic, true, m_isMusicMuted ? 0.f : m_musicVolume);

	g_audio->SetNumListeners(1);
}

void Game::UpdateIntroScreen(float deltaSeconds)
{
	m_timeInState += deltaSeconds;

	if (m_timeInState >= 5.5f)
	{
		g_audio->StopSound(m_introMusicPlayback);
		m_attractMusicPlayback = g_audio->StartSound(m_attractMusic, true, m_musicVolume);
		m_gameState = GameState::ATTRACT;
		m_timeInState = 0.f;
	}

	UpdateCameras();
}

void Game::UpdateAttractScreen(float deltaSeconds)
{
	m_timeInState += deltaSeconds;

	if (g_input->WasKeyJustPressed(KEYCODE_SPACE))
	{
		g_audio->StartSound(m_buttonClickSound);
		UpdatePlayerViewports();
		StartGold();
	}

	if (g_openXR && g_openXR->IsInitialized())
	{
		VRController leftController = g_openXR->GetLeftController();
		if (leftController.WasSelectButtonJustPressed())
		{
			g_audio->StartSound(m_buttonClickSound);
			UpdatePlayerViewports();
			StartGold();
		}
	}

	if (g_input->WasKeyJustPressed(KEYCODE_ESC))
	{
		g_app->HandleQuitRequested();
	}

	Vec2 screenCenter = 0.5f * Vec2(g_gameConfigBlackboard.GetValue("screenSizeX", g_screenSizeX), g_gameConfigBlackboard.GetValue("screenSizeY", g_screenSizeY));

	DebugAddScreenText("Press spacebar to Start Game", screenCenter + Vec2::SOUTH * 350.f, 20.f, Vec2(0.5f, 0.5f), 0.f);
	//DebugAddScreenText("Press START to join with controller", screenCenter + Vec2::SOUTH * 325.f, 20.f, Vec2(0.5f, 0.5f), 0.f);
	DebugAddScreenText("Press Escape to exit", screenCenter + Vec2::SOUTH * 375.f, 20.f, Vec2(0.5f, 0.5f), 0.f);

	UpdateCameras();
}

void Game::UpdateLobby(float)
{
}

void Game::UpdatePlayerViewports()
{
}

void Game::RenderIntroScreen() const
{
	SpriteSheet* logoSpriteSheet = new SpriteSheet(m_logoTexture, IntVec2(15, 19));
	SpriteAnimDefinition logoAnimation(logoSpriteSheet, 0, 271, 2.f, SpriteAnimPlaybackType::ONCE);
	SpriteAnimDefinition logoBlinkAnimation(logoSpriteSheet, 270, 271, 0.2f, SpriteAnimPlaybackType::LOOP);
	
	std::vector<Vertex_PCU> introScreenVertexes;
	std::vector<Vertex_PCU> introScreenFadeOutVertexes;
	std::vector<Vertex_PCU> introScreenTextVerts;
	AABB2 animatedLogoBox(Vec2(g_gameConfigBlackboard.GetValue("screenSizeX", g_screenSizeX), g_gameConfigBlackboard.GetValue("screenSizeY", g_screenSizeY)) * 0.5f - Vec2(320.f, 200.f), Vec2(g_gameConfigBlackboard.GetValue("screenSizeX", g_screenSizeX), g_gameConfigBlackboard.GetValue("screenSizeY", g_screenSizeY)) * 0.5f + Vec2(320.f, 200.f));
	if (m_timeInState >= 2.f)
	{
		SpriteDefinition currentSprite = logoBlinkAnimation.GetSpriteDefAtTime(m_timeInState);
		AddVertsForAABB2(introScreenVertexes, animatedLogoBox, Rgba8::WHITE, currentSprite.GetUVs().m_mins, currentSprite.GetUVs().m_maxs);
	
		int glyphsToDraw = RoundDownToInt((64.f * m_timeInState - 128.f) / 3.f);
		g_squirrelFont->AddVertsForTextInBox2D(introScreenTextVerts, AABB2(Vec2(0.f, 100.f), Vec2(g_screenSizeX, 120.f)), 20.f, "Developed by Shreyas (Rey) Nisal", Rgba8::WHITE, 0.7f, Vec2(0.5f, 0.f), TextBoxMode::OVERRUN, glyphsToDraw);
		g_squirrelFont->AddVertsForTextInBox2D(introScreenTextVerts, AABB2(Vec2(0.f, 70.f), Vec2(g_screenSizeX, 90.f)), 20.f, "Logo by Namita Nisal", Rgba8::WHITE, 0.7f, Vec2(0.5f, 0.f), TextBoxMode::OVERRUN, glyphsToDraw);

		if (m_timeInState >= 4.5f)
		{
			AddVertsForAABB2(introScreenFadeOutVertexes, AABB2(Vec2::ZERO, Vec2(g_gameConfigBlackboard.GetValue("screenSizeX", g_screenSizeX), g_gameConfigBlackboard.GetValue("screenSizeY", g_screenSizeY))), Rgba8(0, 0, 0, static_cast<unsigned char>(RoundDownToInt(RangeMapClamped((5.5f - m_timeInState), 1.f, 0.f, 0.f, 255.f)))));
		}
	}
	else
	{
		SpriteDefinition currentSprite = logoAnimation.GetSpriteDefAtTime(m_timeInState);
		AddVertsForAABB2(introScreenVertexes, animatedLogoBox, Rgba8::WHITE, currentSprite.GetUVs().m_mins, currentSprite.GetUVs().m_maxs);
	}
	g_renderer->BindTexture(m_logoTexture);
	g_renderer->SetBlendMode(BlendMode::ALPHA);
	g_renderer->SetDepthMode(DepthMode::DISABLED);
	g_renderer->SetModelConstants();
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_NONE);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->DrawVertexArray(introScreenVertexes);
	g_renderer->BindShader(nullptr);

	g_renderer->BindTexture(g_squirrelFont->GetTexture());
	g_renderer->DrawVertexArray(introScreenTextVerts);

	g_renderer->BindTexture(nullptr);
	g_renderer->DrawVertexArray(introScreenFadeOutVertexes);
}

void Game::RenderAttractScreen() const
{
	AABB2 screenBox(Vec2::ZERO, Vec2(g_screenSizeX, g_screenSizeY));
	screenBox.AddPadding(-10.f, -75.f);
	AABB2 textBox(screenBox);
	textBox.AddPadding(-10.f, -10.f);

	std::string introText = "After shedding sweat, tears and blood, I somehow made it to the end of SD2.\nHowever, Prof. Butler did not stop coming for me.\nI was given Doomenstein Gold, one final mission.\nI was required to master new weapons, combat new enemies, and accomplish my objective.\n\nHowever, I was tired of fighting billboarded monsters in the creepy lit dungeons, so with Sid's help I stepped outside-\nwith true 3-dimensions and lighting with shadows.\n\nLittle did I know that my rebellion against the requirements was an expected move,\nand Prof. Butler had his army waiting for me.\nIt's now time for this final Gold mission,\nto combat Prof. Butler's army and finally free myself from SD2 once and for all!";
	std::string creditsText = "\n\nCredits\n\nProgrammer\nShreyas (Rey) Nisal\n\nArt Direction\nEric Robles\n\nLogo Design and Animation\nNamita Nisal\n\nArt Assets\nKenney.nl\nQuaternius\n\nMusic and SFX\nopengameart.org\n\nSpecial Thanks\nProf. Matt Butler\nProf. Squirrel Eiserloh\nSiddhant (Sid) Thakur";
	int numGlyphs = RoundDownToInt(m_timeInState * introText.length() / 45.f);
	std::vector<Vertex_PCU> attractScreenTextVerts;
	g_squirrelFont->AddVertsForTextInBox2D(attractScreenTextVerts, textBox, 40.f, introText + creditsText, Rgba8::WHITE, 0.7f, Vec2(0.f, 1.f), TextBoxMode::SHRINK_TO_FIT, numGlyphs);

	Vec2 screenCenter = 0.5f * Vec2(g_gameConfigBlackboard.GetValue("screenSizeX", g_screenSizeX), g_gameConfigBlackboard.GetValue("screenSizeY", g_screenSizeY));

	std::vector<Vertex_PCU> attractScreenBackgroundVerts;
	std::vector<Vertex_PCU> attractScreenVertexes;

	AddVertsForAABB2(attractScreenVertexes, screenBox, Rgba8(0, 0, 0, 185));
	AddVertsForAABB2(attractScreenBackgroundVerts, AABB2(Vec2::ZERO, Vec2(g_screenSizeX, g_screenSizeY)), Rgba8::WHITE);
	g_renderer->SetBlendMode(BlendMode::ALPHA);
	g_renderer->SetDepthMode(DepthMode::DISABLED);
	g_renderer->SetModelConstants();
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_BACK);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->BindShader(nullptr);
	g_renderer->BindTexture(m_attractScreenBackgroundTexture);
	g_renderer->DrawVertexArray(attractScreenBackgroundVerts);
	g_renderer->BindTexture(nullptr);
	g_renderer->DrawVertexArray(attractScreenVertexes);
	g_renderer->BindTexture(g_squirrelFont->GetTexture());
	g_renderer->DrawVertexArray(attractScreenTextVerts);
}

void Game::RenderLobby() const
{
}

void Game::RenderHUD() const
{
}

