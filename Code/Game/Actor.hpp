#pragma once

#include "Game/ActorDefinition.hpp"
#include "Game/ActorUID.hpp"

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Renderer/AnimationGroupDefinition.hpp"


class Map;
struct SpawnInfo;
class Controller;
class StaticActor;

class Actor
{
public:
	virtual ~Actor();
	Actor() = default;
	Actor(Map* map, SpawnInfo const& spawnInfo, ActorUID uid);

	virtual void				Update();
	virtual void				UpdatePhysics();
	virtual void				Render() const;
	void						RenderDebug() const;

	virtual void				TakeDamage(float damage);
	virtual void				Die();

	virtual void				AddForce(Vec3 const& force);
	virtual void				AddImpulse(Vec3 const& impulse);
	virtual void				OnCollide(Actor* other);
	virtual void				OnCollideWithStatic(StaticActor* other);
	
	virtual void				OnPossessed(Controller* controller);
	virtual void				OnUnpossessed();
	virtual void				MoveInDirection(Vec3 const& direction, float speed);
	virtual void				TurnInDirection(float targetOrientation, float maxTurnRate);
	
	virtual Weapon const*		GetEquippedWeapon() const;
	virtual void				EquipWeapon(int weaponIndex);
	virtual void				EquipNextWeapon();
	virtual void				EquipPreviousWeapon();
	virtual void				Attack();

	virtual Mat44 const			GetModelMatrix() const;
	virtual Mat44 const			GetRenderModelMatrix() const;
	virtual Vec3 const			GetForwardNormal() const;
	virtual Vec3 const			GetLeftNormal() const;
	virtual Vec3 const			GetUpNormal() const;
	virtual Vec3 const			GetEyePosition() const;
	Vec3 const					GetWeaponPosition() const;

public:
	ActorUID					m_UID = ActorUID::INVALID;
	ActorDefinition				m_definition;
	Map*						m_map = nullptr;
	Vec3						m_position = Vec3::ZERO;
	Vec3						m_pivotPosition = Vec3::ZERO;
	EulerAngles					m_orientation = EulerAngles::ZERO;
	Vec3						m_velocity = Vec3::ZERO;
	Vec3						m_acceleration = Vec3::ZERO;
	std::vector<Vertex_PCU>		m_unlitVertexes;
	std::vector<Vertex_PCUTBN>	m_litVertexes;
	std::vector<unsigned int>	m_indexes;
	std::vector<Weapon*>		m_weapons;
	std::vector<Weapon*>		m_leftWeapons;
	std::vector<Weapon*>		m_rightWeapons;
	int							m_equippedWeaponIndex = -1;
	ActorUID					m_ownerUID = ActorUID::INVALID;

	float						m_physicsRadius = 0.f;
	float						m_physicsHeight = 0.f;
	bool						m_isStatic = false;
	bool						m_isDead = false;
	bool						m_isDestroyed = false;
	float						m_health = 200.f;
	Stopwatch					m_lifetimeTimer;

	Controller*					m_controller;
	Controller*					m_aiController;

	Clock						m_animationClock;
	AnimationGroupDefinition	m_currentAnimation;

	SoundPlaybackID				m_hurtSoundPlayback;
	bool						m_isGrounded = false;

};