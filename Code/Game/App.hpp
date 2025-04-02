#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Renderer/Camera.hpp"

#include "Game/Game.hpp"

class App
{
public:
						App							();
						~App						();
	void				Startup						();
	void				Shutdown					();
	void				Run							();
	void				RunFrame					();

	bool				IsQuitting					() const		{ return m_isQuitting; }
	bool				HandleQuitRequested			();
	static bool			HandleQuitRequested			(EventArgs& args);
	static bool			ShowControls				(EventArgs& args);

	// VR Integration
	XREye				GetCurrentEye() const;
	Camera const		GetCurrentCamera() const;
	void				InitializeCameras			();
	void				RenderScreen() const;
	void				RenderCustomScreens() const;

public:
	Game*				m_game;
	float				m_frameRate;

	// VR Integration
	Camera				m_worldCamera;
	Camera				m_screenCamera;
	Camera				m_leftEyeCamera;
	Camera				m_rightEyeCamera;
	XREye				m_currentEye				= XREye::NONE;
	Texture*			m_screenRTVTexture = nullptr;

private:
	void				BeginFrame					();
	void				Update						();
	void				Render						() const;
	void				EndFrame					();

	void				LoadGameConfigXml			();

private:
	bool				m_isQuitting				= false;

	Camera				m_devConsoleCamera			= Camera();
};
