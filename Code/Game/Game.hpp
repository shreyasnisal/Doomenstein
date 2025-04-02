#pragma once

#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Renderer/Renderer.hpp"

#include "Game/GameCommon.hpp"

class		App;
class		Entity;
class		Player;
class		Prop;
class		Texture;
class		Map;
class		GoldMap;

enum class GameState
{
	INTRO,
	ATTRACT,
	LOBBY,
	GAME
};

class Game
{
public:
	~Game();
	Game();
	void						Update												();
	void						Render												() const;
	void						RenderCustomScreens									() const;
	void						RenderScreen										() const;

	void						GoToLobby();
	void						QuitToLobby();

	void						StartGame											();
	void						QuitToAttractScreen									();
	void						StartGold();
	
public:	
	static constexpr float SCREEN_QUAD_DISTANCE = 2.f;

	bool						m_drawDebug											= false;

	//Camera						m_screenCamera;

	//std::vector<Player*>		m_players;
	Player* m_player;

	SoundPlaybackID				m_introMusicPlayback;
	SoundID						m_attractMusic = MISSING_SOUND_ID;
	SoundID						m_gameMusic = MISSING_SOUND_ID;
	SoundID						m_buttonClickSound = MISSING_SOUND_ID;
	float						m_musicVolume = 0.1f;
	SoundPlaybackID				m_gameMusicPlayback;
	SoundPlaybackID				m_attractMusicPlayback;

	// Change gameState to INTRO to show intro screen
	// BEWARE: The intro screen spritesheet is a large texture and the game will take much longer to load
	GameState					m_gameState											= GameState::ATTRACT;

	Clock						m_gameClock = Clock();
	Map*						m_currentMap = nullptr;

	Vec3						m_sunDirection = Vec3(2.f, -1.f, -1.f);
	float						m_sunIntensity = 0.9f;
	float						m_ambientIntensity = 0.1f;

	bool						m_isMusicMuted = false;
	bool						m_isSFXMuted = false;
	Mat44						m_screenBillboardMatrix = Mat44::IDENTITY;

private:

	void						UpdateIntroScreen									(float deltaSeconds);
	void						UpdateAttractScreen									(float deltaSeconds);
	void						UpdateLobby											(float deltaSeconds);
	void						UpdateGame											(float deltaSeconds);
	void						UpdatePlayers										(float deltaSeconds);
	void						UpdatePlayerViewports								();
	void						UpdateCameras										();

	void						HandleDeveloperCheats								();

	void						RenderIntroScreen									() const;
	void						RenderAttractScreen									() const;
	void						RenderLobby											() const;
	void						RenderGame											() const;
	void						RenderGameScreen									() const;
	void						RenderWorldScreenQuad								() const;

	void						RenderHUD											() const;

	void						LoadAssets											();

private:
	Texture*					m_testTexture										= nullptr;
	Texture*					m_logoTexture										= nullptr;
	Texture*					m_attractScreenBackgroundTexture					= nullptr;
	float						m_timeInState										= 0.f;
};