#include "Game/Player.hpp"

#include "Game/App.hpp"
#include "Game/Actor.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/Weapon.hpp"

#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/VirtualReality/OpenXR.hpp"


bool Player::Event_Cursor(EventArgs& args)
{
	bool help = args.GetValue("help", false);
	if (help)
	{
		g_console->AddLine("Modifies settings for the mouse cursor", false);
		g_console->AddLine("Parameters", false);
		g_console->AddLine(Stringf("\t\t%-20s: [float > 0.f] changes the cursor sentivity to the specified value", "sensitivity"), false);
	}
	else
	{
		float sensitivity = args.GetValue("sensitivity", 1.f);
		if (sensitivity > 0.f)
		{
			g_app->m_game->m_player->SetCursorSensitivity(sensitivity);
		}
	}

	return true;
}

Player::Player(Game* game, int playerIndex, int controllerIndex)
	: m_game(game)
	, m_playerIndex(playerIndex)
	, m_controllerIndex(controllerIndex)
{
	//g_app->m_worldCamera.SetRenderBasis(Vec3::SKYWARD, Vec3::WEST, Vec3::NORTH);

	SubscribeEventCallbackFunction("Cursor", Event_Cursor, "Change settings for the mouse cursor");

	//g_app->m_screenCamera.SetOrthoView(Vec2::ZERO, Vec2(g_screenSizeX, g_screenSizeY));

	//g_app->m_worldCamera.SetPerspectiveView(GetViewportAspect(), 60.f, 0.01f, 1000.f);
}

void Player::Update()
{
	if (!m_game->m_currentMap)
	{
		return;
	}

	Actor* possessedActor = m_game->m_currentMap->GetActorByUID(m_actorUID);

	if (g_input->WasKeyJustPressed('F'))
	{
		m_freeFlyMode = !m_freeFlyMode;
		if (m_freeFlyMode)
		{
			Actor* currentActor = m_game->m_currentMap->GetActorByUID(m_actorUID);
			if (currentActor)
			{
				Unpossess(currentActor);
			}
		}
		else
		{
			Actor* possessingActor = m_game->m_currentMap->GetActorByUID(m_lastPossessedActorUID);
			if (possessingActor)
			{
				Possess(possessingActor);
			}
		}
	}

	if (m_freeFlyMode)
	{
		UpdateFreeFlyInput();
	}

	if (!m_game->m_gameClock.IsPaused())
	{
		if (possessedActor)
		{
			UpdateFirstPersonInput();
		}
	}

	if (m_freeFlyMode)
	{
		m_orientation.m_rollDegrees = GetClamped(m_orientation.m_rollDegrees, -45.f, 45.f);
		m_orientation.m_pitchDegrees = GetClamped(m_orientation.m_pitchDegrees, -85.f, 85.f);
	}

	g_app->m_worldCamera.SetTransform(m_position, m_orientation);
	g_audio->UpdateListeners(m_playerIndex, m_position, GetForwardNormal(), GetUpNormal());
}

void Player::UpdateFreeFlyInput()
{
	UpdateFreeFlyKeyboardAndMouseInput();
	UpdateFreeFlyControllerInput();
}

void Player::UpdateFreeFlyKeyboardAndMouseInput()
{
	float deltaSeconds = Clock::GetSystemClock().GetDeltaSeconds();

	constexpr float MOVEMENT_SPEED = 2.f;
	constexpr float TURN_RATE_PER_MOUSE_CLIENT_DELTA = 0.075f;
	constexpr float ROLL_RATE = 15.f;
	float sprintFactor = g_input->IsShiftHeld() ? 10.f : 1.f;

	Vec3 playerForward;
	Vec3 playerLeft;
	Vec3 playerUp;
	m_orientation.GetAsVectors_iFwd_jLeft_kUp(playerForward, playerLeft, playerUp);

	if (g_input->IsKeyDown('W'))
	{
		m_position += playerForward * MOVEMENT_SPEED * sprintFactor * deltaSeconds;
	}
	if (g_input->IsKeyDown('S'))
	{
		m_position -= playerForward * MOVEMENT_SPEED * sprintFactor * deltaSeconds;
	}
	if (g_input->IsKeyDown('A'))
	{
		m_position += playerLeft * MOVEMENT_SPEED * sprintFactor * deltaSeconds;
	}
	if (g_input->IsKeyDown('D'))
	{
		m_position -= playerLeft * MOVEMENT_SPEED * sprintFactor * deltaSeconds;
	}
	if (g_input->IsKeyDown('C'))
	{
		m_position += Vec3::SKYWARD * MOVEMENT_SPEED * sprintFactor * deltaSeconds;
	}
	if (g_input->IsKeyDown('Z'))
	{
		m_position += Vec3::GROUNDWARD * MOVEMENT_SPEED * sprintFactor * deltaSeconds;
	}
	if (g_input->IsKeyDown('Q'))
	{
		m_orientation.m_rollDegrees -= ROLL_RATE * deltaSeconds;
	}
	if (g_input->IsKeyDown('E'))
	{
		m_orientation.m_rollDegrees += ROLL_RATE * deltaSeconds;
	}

	if (g_input->WasKeyJustPressed(KEYCODE_ESC))
	{
		m_game->QuitToAttractScreen();
	}

	if (g_input->WasKeyJustPressed(KEYCODE_LMB))
	{
		DoomRaycastResult raycastResult = m_game->m_currentMap->RaycastVsAll(m_position, GetForwardNormal(), 10.f);
		DebugAddWorldLine(raycastResult.m_rayStartPosition, raycastResult.m_impactPosition, 0.01f, 10.f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::X_RAY);
		if (raycastResult.m_didImpact)
		{
			DebugAddWorldPoint(raycastResult.m_impactPosition, 0.06f, 10.f, Rgba8::WHITE, Rgba8::WHITE);
			DebugAddWorldArrow(raycastResult.m_impactPosition, raycastResult.m_impactPosition + raycastResult.m_impactNormal * 0.3f, 0.01f, 10.f, Rgba8::BLUE, Rgba8::BLUE);
		}
	}
	if (g_input->WasKeyJustPressed(KEYCODE_RMB))
	{
		DoomRaycastResult raycastResult = m_game->m_currentMap->RaycastVsAll(m_position, GetForwardNormal(), 0.25f);
		DebugAddWorldLine(raycastResult.m_rayStartPosition, raycastResult.m_impactPosition, 0.01f, 10.f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::X_RAY);
		if (raycastResult.m_didImpact)
		{
			DebugAddWorldPoint(raycastResult.m_impactPosition, 0.06f, 10.f, Rgba8::WHITE, Rgba8::WHITE);
			DebugAddWorldArrow(raycastResult.m_impactPosition, raycastResult.m_impactPosition + raycastResult.m_impactNormal * 0.3f, 0.01f, 10.f, Rgba8::BLUE, Rgba8::BLUE);
		}
	}

	m_orientation.m_yawDegrees += g_input->GetCursorClientDelta().x * TURN_RATE_PER_MOUSE_CLIENT_DELTA * m_cursorSensitivity;
	m_orientation.m_pitchDegrees -= g_input->GetCursorClientDelta().y * TURN_RATE_PER_MOUSE_CLIENT_DELTA * m_cursorSensitivity;
}

void Player::UpdateFreeFlyControllerInput()
{
	float deltaSeconds = m_game->m_gameClock.GetDeltaSeconds();

	constexpr float MOVEMENT_SPEED = 2.f;
	constexpr float TURN_RATE = 90.f;
	Vec3 playerForward;
	Vec3 playerLeft;
	Vec3 playerUp;
	m_orientation.GetAsVectors_iFwd_jLeft_kUp(playerForward, playerLeft, playerUp);

	XboxController controller = g_input->GetController(0);
	AnalogJoystick leftStick = controller.GetLeftStick();
	AnalogJoystick rightStick = controller.GetRightStick();
	float sprintFactor = controller.IsButtonDown(XBOX_BUTTON_A) ? 10.f : 1.f;

	Vec2 velocityXY = Vec2::MakeFromPolarDegrees(leftStick.GetOrientationDegrees(), leftStick.GetMagnitude());
	velocityXY.RotateMinus90Degrees();
	float velocityZ = 0.f;
	if (controller.IsButtonDown(XBOX_BUTTON_RIGHT_SHOULDER))
	{
		velocityZ += 1.f;
	}
	if (controller.IsButtonDown(XBOX_BUTTON_LEFT_SHOULDER))
	{
		velocityZ -= 1.f;
	}
	m_position += playerForward * velocityXY.x * MOVEMENT_SPEED * sprintFactor * deltaSeconds;
	m_position += playerLeft * velocityXY.y * MOVEMENT_SPEED * sprintFactor * deltaSeconds;
	m_position += Vec3::SKYWARD * velocityZ * MOVEMENT_SPEED * sprintFactor * deltaSeconds;

	m_orientation.m_yawDegrees += -rightStick.GetPosition().x * TURN_RATE * deltaSeconds;
	m_orientation.m_pitchDegrees -= rightStick.GetPosition().y * TURN_RATE * deltaSeconds;
	float roll = 0.f;
	roll += controller.GetRightTrigger();
	roll -= controller.GetLeftTrigger();
	m_orientation.m_rollDegrees += roll * TURN_RATE * deltaSeconds;
}

void Player::UpdateFirstPersonInput()
{
	UpdateFirstPersonKeyboardAndMouseInput();
	if (g_openXR && g_openXR->IsInitialized())
	{
		UpdateFirstPersonVRInput();
	}
}

void Player::UpdateFirstPersonKeyboardAndMouseInput()
{
	constexpr float TURN_RATE_PER_MOUSE_CLIENT_DELTA = 0.075f;
	Actor* possessedActor = m_game->m_currentMap->GetActorByUID(m_actorUID);

	if (!possessedActor)
	{
		return;
	}

	float movementSpeed = g_input->IsShiftHeld() ? possessedActor->m_definition.m_runSpeed : possessedActor->m_definition.m_walkSpeed;
	Vec3 movementFwd = possessedActor->GetModelMatrix().GetIBasis3D();
	Vec3 movementLeft = possessedActor->GetModelMatrix().GetJBasis3D();

	if (g_input->WasKeyJustPressed(KEYCODE_ESC))
	{
		m_game->QuitToAttractScreen();
	}
	if (g_input->IsKeyDown('W'))
	{
		possessedActor->MoveInDirection(movementFwd, movementSpeed);
	}
	if (g_input->IsKeyDown('S'))
	{
		possessedActor->MoveInDirection(-movementFwd, movementSpeed);
	}
	if (g_input->IsKeyDown('A'))
	{
		possessedActor->MoveInDirection(movementLeft, movementSpeed);
	}
	if (g_input->IsKeyDown('D'))
	{
		possessedActor->MoveInDirection(-movementLeft, movementSpeed);
	}
	if (g_input->IsKeyDown(KEYCODE_LMB))
	{
		possessedActor->Attack();
	}
	if (g_input->WasKeyJustPressed(KEYCODE_SPACE) && possessedActor->m_isGrounded)
	{
		possessedActor->AddImpulse(Vec3::SKYWARD * GRAVITY * 0.75f);
		possessedActor->m_isGrounded = false;
	}
	if (g_input->WasKeyJustPressed('1'))
	{
		possessedActor->EquipWeapon(0);
	}
	if (g_input->WasKeyJustPressed('2'))
	{
		possessedActor->EquipWeapon(1);
	}
	if (g_input->WasKeyJustPressed('3'))
	{
		possessedActor->EquipWeapon(2);
	}
	if (g_input->WasKeyJustPressed('Q'))
	{
		possessedActor->EquipPreviousWeapon();
	}
	if (g_input->WasKeyJustPressed('E'))
	{
		possessedActor->EquipNextWeapon();
	}

	possessedActor->m_orientation.m_yawDegrees += g_input->GetCursorClientDelta().x * TURN_RATE_PER_MOUSE_CLIENT_DELTA * m_cursorSensitivity;
	possessedActor->m_orientation.m_pitchDegrees -= g_input->GetCursorClientDelta().y * TURN_RATE_PER_MOUSE_CLIENT_DELTA * m_cursorSensitivity;

	//m_position = possessedActor->GetEyePosition();
	//possessedActor->m_orientation.m_yawDegrees = m_orientation.m_yawDegrees;
	//possessedActor->m_orientation.m_pitchDegrees = m_orientation.m_pitchDegrees;
}

void Player::UpdateFirstPersonControllerInput()
{
	Actor* possessedActor = m_game->m_currentMap->GetActorByUID(m_actorUID);
	if (!possessedActor)
	{
		return;
	}

	float deltaSeconds = m_game->m_gameClock.GetDeltaSeconds();

	XboxController controller = g_input->GetController(m_controllerIndex);
	AnalogJoystick leftStick = controller.GetLeftStick();
	AnalogJoystick rightStick = controller.GetRightStick();

	float movementSpeed = controller.IsButtonDown(XBOX_BUTTON_A) ? possessedActor->m_definition.m_runSpeed : possessedActor->m_definition.m_walkSpeed;
	Vec3 movementFwd = possessedActor->GetModelMatrix().GetIBasis3D();
	Vec3 movementLeft = possessedActor->GetModelMatrix().GetJBasis3D();

	if (controller.GetRightTrigger() > 0.f)
	{
		possessedActor->Attack();
	}
	if (controller.WasButtonJustPressed(XBOX_BUTTON_X))
	{
		possessedActor->EquipWeapon(0);
	}
	if (controller.WasButtonJustPressed(XBOX_BUTTON_Y))
	{
		possessedActor->EquipWeapon(1);
	}
	if (controller.WasButtonJustPressed(XBOX_BUTTON_BACK))
	{
		m_game->QuitToLobby();
	}

	Vec2 velocityXY = Vec2::MakeFromPolarDegrees(leftStick.GetOrientationDegrees(), leftStick.GetMagnitude());
	velocityXY.RotateMinus90Degrees();

	possessedActor->m_position += movementFwd * velocityXY.x * movementSpeed * deltaSeconds;
	possessedActor->m_position += movementLeft * velocityXY.y * movementSpeed * deltaSeconds;

	possessedActor->m_orientation.m_yawDegrees += -rightStick.GetPosition().x * possessedActor->m_definition.m_turnSpeed * deltaSeconds;
	possessedActor->m_orientation.m_pitchDegrees -= rightStick.GetPosition().y * possessedActor->m_definition.m_turnSpeed * deltaSeconds;
	
	//m_position = possessedActor->GetEyePosition();
	//possessedActor->m_orientation.m_yawDegrees = m_orientation.m_yawDegrees;
	//possessedActor->m_orientation.m_pitchDegrees = m_orientation.m_pitchDegrees;
}

void Player::UpdateFirstPersonVRInput()
{
	if (!m_game->m_currentMap)
	{
		return;
	}

	float deltaSeconds = m_game->m_gameClock.GetDeltaSeconds();

	VRController const& leftController = g_openXR->GetLeftController();
	VRController const& rightController = g_openXR->GetRightController();

	AnalogJoystick leftStick = leftController.GetJoystick();
	AnalogJoystick rightStick = rightController.GetJoystick();

	Actor* possessedActor = m_game->m_currentMap->GetActorByUID(m_actorUID);
	if (!possessedActor)
	{
		return;
	}

	float movementSpeed = rightController.IsBackButtonPressed() ? possessedActor->m_definition.m_runSpeed : possessedActor->m_definition.m_walkSpeed;
	Vec3 movementFwd = possessedActor->GetModelMatrix().GetIBasis3D();
	Vec3 movementLeft = possessedActor->GetModelMatrix().GetJBasis3D();

	if (leftController.GetTrigger() > 0.f)
	{
		possessedActor->m_leftWeapons[m_leftWeaponIndex]->Fire();;
	}
	if (leftController.WasGripJustPressed())
	{
		m_leftWeaponIndex = (m_leftWeaponIndex + 1) % (int)possessedActor->m_leftWeapons.size();
		possessedActor->m_leftWeapons[m_leftWeaponIndex]->OnEquipped(possessedActor);
	}

	if (rightController.GetTrigger() > 0.f)
	{
		possessedActor->m_rightWeapons[m_rightWeaponIndex]->Fire();;
	}
	if (rightController.WasGripJustPressed())
	{
		m_rightWeaponIndex = (m_rightWeaponIndex + 1) % (int)possessedActor->m_rightWeapons.size();
		possessedActor->m_rightWeapons[m_rightWeaponIndex]->OnEquipped(possessedActor);
	}

	if (rightController.WasSelectButtonJustPressed())
	{
		possessedActor->AddImpulse(Vec3::SKYWARD * GRAVITY * 0.75f);
		possessedActor->m_isGrounded = false;
	}

	Vec2 velocityXY = Vec2::MakeFromPolarDegrees(leftStick.GetOrientationDegrees(), leftStick.GetMagnitude());
	velocityXY.RotateMinus90Degrees();

	possessedActor->m_position += movementFwd * velocityXY.x * movementSpeed * deltaSeconds;
	possessedActor->m_position += movementLeft * velocityXY.y * movementSpeed * deltaSeconds;

	possessedActor->m_orientation.m_yawDegrees -= rightStick.GetPosition().x * possessedActor->m_definition.m_turnSpeed * deltaSeconds;
	//possessedActor->m_orientation.m_pitchDegrees -= rightStick.GetPosition().y * possessedActor->m_definition.m_turnSpeed * deltaSeconds;

	Mat44 playerModelMatrix = Mat44::CreateTranslation3D(m_position);
	playerModelMatrix.Append(m_orientation.GetAsMatrix_iFwd_jLeft_kUp());

	m_rightControllerWorldPosition = playerModelMatrix.TransformPosition3D(rightController.GetPosition_iFwd_jLeft_kUp());
	m_rightControllerOrientation = m_orientation + rightController.GetOrientation_iFwd_jLeft_kUp();
	m_leftControllerWorldPosition = playerModelMatrix.TransformPosition3D(leftController.GetPosition_iFwd_jLeft_kUp());
	m_leftControllerOrientation = m_orientation + leftController.GetOrientation_iFwd_jLeft_kUp();
}

void Player::RenderScreen() const
{
	if (!m_game->m_currentMap)
	{
		return;
	}

	//g_renderer->BeginCamera(g_app->m_screenCamera);

	std::vector<Vertex_PCU> hudVerts;
	std::vector<Vertex_PCU> weaponVerts;
	std::vector<Vertex_PCU> reticleVerts;
	std::vector<Vertex_PCU> screenTextVerts;


	Actor* possessedActor = m_game->m_currentMap->GetActorByUID(m_actorUID);
	if (!possessedActor)
	{
		return;
	}

	AABB2 screenBox(GetNormalizedScreenCoordinates().m_mins * Vec2(g_screenSizeX, g_screenSizeY), GetNormalizedScreenCoordinates().m_maxs * Vec2(g_screenSizeX, g_screenSizeY));

	if (possessedActor->m_definition.m_is3DActor)
	{
		std::vector<Vertex_PCU> healthBarVerts;
		AABB2 healthBarOuterBounds = AABB2(Vec2(30.f, g_screenSizeY - 50.f), Vec2(230.f, g_screenSizeY - 30.f));
		AABB2 healthBarInnerBounds(healthBarOuterBounds);
		healthBarInnerBounds.AddPadding(-4.f, -2.f);

		AddVertsForAABB2(healthBarVerts, healthBarOuterBounds, Rgba8::WHITE);

		float healthFraction = possessedActor->m_health / possessedActor->m_definition.m_health;
		AddVertsForAABB2(healthBarVerts, healthBarInnerBounds, Rgba8::RED);
		
		AABB2 healthBarBounds(healthBarInnerBounds);
		healthBarBounds.m_maxs.x *= healthFraction;
		AddVertsForAABB2(healthBarVerts, healthBarBounds, Rgba8::GREEN);
		g_renderer->DrawVertexArray(healthBarVerts);

		Vec2 screenCenter = screenBox.GetCenter();
		AddVertsForAABB2(reticleVerts, AABB2(screenCenter - possessedActor->m_weapons[possessedActor->m_equippedWeaponIndex]->m_definition.m_reticleSize.GetAsVec2() * 0.5f, screenCenter + possessedActor->m_weapons[possessedActor->m_equippedWeaponIndex]->m_definition.m_reticleSize.GetAsVec2() * 0.5f), Rgba8::WHITE);
		g_renderer->BindTexture(possessedActor->m_weapons[possessedActor->m_equippedWeaponIndex]->m_definition.m_reticleTexture);
		g_renderer->DrawVertexArray(reticleVerts);

		g_renderer->EndCamera(g_app->m_screenCamera);

		return;
	}

	AddVertsForAABB2(hudVerts, screenBox.GetBoxAtUVs(Vec2::ZERO, Vec2(1.f, 0.128f / GetNormalizedScreenCoordinates().GetDimensions().y)), Rgba8::WHITE);

	g_renderer->SetBlendMode(BlendMode::OPAQUE);
	g_renderer->SetDepthMode(DepthMode::DISABLED);
	g_renderer->SetModelConstants();
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_BACK);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->BindShader(possessedActor->m_weapons[possessedActor->m_equippedWeaponIndex]->m_definition.m_hudShader);
	g_renderer->BindTexture(possessedActor->m_weapons[possessedActor->m_equippedWeaponIndex]->m_definition.m_hudTexture);
	g_renderer->DrawVertexArray(hudVerts);

	Weapon* const& weapon = possessedActor->m_weapons[possessedActor->m_equippedWeaponIndex];

	if (weapon->m_currentAnimation.GetDuration() < weapon->m_animationClock->GetTotalSeconds())
	{
		weapon->m_currentAnimation = weapon->m_definition.m_idleAnimation;
	}

	float weaponScalingFactor = (GetNormalizedScreenCoordinates().GetDimensions().x * GetNormalizedScreenCoordinates().GetDimensions().y);
	SpriteAnimDefinition currentWeaponAnimation = weapon->m_currentAnimation;
	SpriteDefinition sprite = currentWeaponAnimation.GetSpriteDefAtTime(weapon->m_animationClock->GetTotalSeconds());
	Vec2 weaponBottomCenter = screenBox.GetPointAtUV(Vec2(0.5f, 0.128f / GetNormalizedScreenCoordinates().GetDimensions().y));
	Vec2 weaponBottomLeft = weaponBottomCenter - Vec2((float)weapon->m_definition.m_spriteSize.x * 0.5f * weaponScalingFactor, 0.f);
	Vec2 weaponTopRight = weaponBottomCenter + Vec2((float)weapon->m_definition.m_spriteSize.x * 0.5f * weaponScalingFactor, (float)weapon->m_definition.m_spriteSize.y * weaponScalingFactor);
	AddVertsForAABB2(weaponVerts, AABB2(weaponBottomLeft, weaponTopRight), Rgba8::WHITE, sprite.GetUVs().m_mins, sprite.GetUVs().m_maxs);

	g_renderer->BindShader(weapon->m_definition.m_idleAnimationShader);
	g_renderer->BindTexture(weapon->m_definition.m_idleAnimation.GetTexture());
	g_renderer->DrawVertexArray(weaponVerts);

	Vec2 screenCenter = screenBox.GetCenter();
	AddVertsForAABB2(reticleVerts, AABB2(screenCenter - weapon->m_definition.m_reticleSize.GetAsVec2() * 0.5f, screenCenter + weapon->m_definition.m_reticleSize.GetAsVec2() * 0.5f), Rgba8::WHITE);
	g_renderer->BindTexture(weapon->m_definition.m_reticleTexture);
	g_renderer->DrawVertexArray(reticleVerts);

	int renderedHealth = RoundDownToInt(GetClamped(possessedActor->m_health, 0.f, possessedActor->m_definition.m_health));
	g_squirrelFont->AddVertsForTextInBox2D(screenTextVerts, screenBox.GetBoxAtUVs(Vec2(0.f, 0.f), Vec2(0.15f, 0.128f / GetNormalizedScreenCoordinates().GetDimensions().y)), 40.f, Stringf("%d", m_kills).c_str(), Rgba8::WHITE, 0.7f, Vec2(0.5f, 0.5f));
	g_squirrelFont->AddVertsForTextInBox2D(screenTextVerts, screenBox.GetBoxAtUVs(Vec2(0.25f, 0.f), Vec2(0.36f, 0.128f / GetNormalizedScreenCoordinates().GetDimensions().y)), 40.f, Stringf("%d", renderedHealth).c_str(), Rgba8::WHITE, 0.7f, Vec2(0.5f, 0.5f));
	g_squirrelFont->AddVertsForTextInBox2D(screenTextVerts, screenBox.GetBoxAtUVs(Vec2(0.85f, 0.f), Vec2(1.f, 0.128f / GetNormalizedScreenCoordinates().GetDimensions().y)), 40.f, Stringf("%d", m_deaths).c_str(), Rgba8::WHITE, 0.7f, Vec2(0.5f, 0.5f));

	g_renderer->SetBlendMode(BlendMode::ALPHA);
	g_renderer->BindTexture(g_squirrelFont->GetTexture());
	g_renderer->DrawVertexArray(screenTextVerts);

	//g_renderer->EndCamera(g_app->m_screenCamera);
}

Vec3 const Player::GetForwardNormal() const
{
	return m_orientation.GetAsMatrix_iFwd_jLeft_kUp().GetIBasis3D();
}

Vec3 const Player::GetLeftNormal() const
{
	return m_orientation.GetAsMatrix_iFwd_jLeft_kUp().GetJBasis3D();
}

Vec3 const Player::GetUpNormal() const
{
	return m_orientation.GetAsMatrix_iFwd_jLeft_kUp().GetKBasis3D();
}

void Player::SetCursorSensitivity(float sensitivity)
{
	m_cursorSensitivity = sensitivity;
}

void Player::DamagedBy(Actor* actor)
{
	if (!actor)
	{
		return;
	}

	Actor* possessedActor = m_game->m_currentMap->GetActorByUID(m_actorUID);
	if (!possessedActor)
	{
		return;
	}
	if (possessedActor->m_isDead)
	{
		KilledBy(actor);

		if (actor->m_controller)
		{
			actor->m_controller->Killed(possessedActor);
		}
	}
}

void Player::KilledBy(Actor* actor)
{
	UNUSED(actor);
	m_deaths++;
	Actor* possessedActor = m_game->m_currentMap->GetActorByUID(m_actorUID);
	if (!possessedActor)
	{
		return;
	}
	possessedActor->Die();
}

void Player::Killed(Actor* actor)
{
	if (actor->m_controller && actor->m_controller->IsPlayer())
	{
		m_kills++;
	}
	actor->Die();
}

void Player::Possess(Actor* actor)
{
	Controller::Possess(actor);
	m_map = actor->m_map;
	m_position = actor->GetEyePosition();
	m_orientation = actor->m_orientation;

	//g_app->m_worldCamera.SetPerspectiveView(GetViewportAspect(), actor->m_definition.m_eyeFov, 0.01f, 1000.f);
}

void Player::Unpossess(Actor* actor)
{
	m_lastPossessedActorUID = actor->m_UID;
	Controller::Unpossess(actor);
}

void Player::SetNormalizedViewport(Vec2 const& normalizedViewportBottomLeft, Vec2 const& normalizedViewportDimensions)
{
	g_app->m_worldCamera.SetNormalizedViewport(normalizedViewportBottomLeft, normalizedViewportDimensions);
}

AABB2 Player::GetNormalizedScreenCoordinates() const
{
	return AABB2(g_app->m_worldCamera.GetNormalizedViewportBottomLeft(), g_app->m_worldCamera.GetNormalizedViewportTopRight());
}

float Player::GetViewportAspect() const
{
	Vec2 normalizedDimensions = g_app->m_worldCamera.GetNormalizedViewportDimensions();
	return g_window->GetAspect() * normalizedDimensions.x / normalizedDimensions.y;
}
