#pragma once

#include "Game/ActorUID.hpp"
#include "Game/Controller.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"


class Actor;
class Game;

class Player : public Controller
{
public:
	~Player() = default;
	//Player(Map* map, Vec3 const& startPosition, EulerAngles const& startOrientation);
	Player(Game* game, int playerIndex, int controllerIndex);

	virtual void Update() override;
	void UpdateFirstPersonInput();
	void UpdateFirstPersonKeyboardAndMouseInput();
	void UpdateFirstPersonControllerInput();
	void UpdateFirstPersonVRInput();
	void UpdateFreeFlyInput();
	void UpdateFreeFlyKeyboardAndMouseInput();
	void UpdateFreeFlyControllerInput();

	void RenderScreen() const;

	virtual bool IsPlayer() const override { return true; }
	virtual void DamagedBy(Actor* actor) override;
	virtual void KilledBy(Actor* actor) override;
	virtual void Killed(Actor* actor) override;

	virtual void Possess(Actor* actor) override;
	virtual void Unpossess(Actor* actor) override;

	Vec3 const				GetForwardNormal() const;
	Vec3 const				GetLeftNormal() const;
	Vec3 const				GetUpNormal() const;

	void SetCursorSensitivity(float sensitivity);
	static bool Event_Cursor(EventArgs& args);

	void SetNormalizedViewport(Vec2 const& normalizedViewportBottomLeft, Vec2 const& normalizedViewportDimensions);
	AABB2 GetNormalizedScreenCoordinates() const;
	float GetViewportAspect() const;

public:
	//Camera					m_worldCamera = Camera();
	//Camera					m_screenCamera = Camera();
	Game*					m_game = nullptr;
	Vec3					m_position;
	Vec3					m_velocity;
	EulerAngles				m_orientation;
	float					m_cursorSensitivity = 1.f;

	bool					m_freeFlyMode = false;

	int						m_playerIndex = 0;
	int						m_controllerIndex = -1;

	int						m_kills = 0;
	int						m_deaths = 0;
	ActorUID				m_lastPossessedActorUID = ActorUID::INVALID;

	// VR
	Vec3 m_leftEyeLocalPosition = Vec3::ZERO;
	Vec3 m_rightEyeLocalPosition = Vec3::ZERO;
	EulerAngles m_hmdOrientation = EulerAngles::ZERO;

	Vec3 m_leftControllerWorldPosition = Vec3::ZERO;
	Vec3 m_rightControllerWorldPosition = Vec3::ZERO;
	EulerAngles m_leftControllerOrientation = EulerAngles::ZERO;
	EulerAngles m_rightControllerOrientation = EulerAngles::ZERO;

	int m_leftWeaponIndex = 0;
	int m_rightWeaponIndex = 0;
};
