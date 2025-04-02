#pragma once

#include "Game/ActorDefinition.hpp"

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Math/FloatRange.hpp"

struct WeaponDefinition
{
public:
	std::string					m_name = "";
	float						m_refireTime = 0.f;
	int							m_rayCount = 0;
	float						m_rayCone = 0.f;
	float						m_rayRange = 0.f;
	FloatRange					m_rayDamage = FloatRange(0.f, 1.f);
	float						m_rayImpulse = 0.f;

	int							m_projectileCount = 0;
	std::string					m_projectileActor;
	float						m_projectileCone = 0.f;
	float						m_projectileSpeed = 0.f;
	
	int							m_meleeCount = 0;
	float						m_meleeArc = 90.f;
	float						m_meleeRange = 0.f;
	FloatRange					m_meleeDamage = FloatRange(0.f, 1.f);
	float						m_meleeImpulse = 0.f;

	Texture*					m_hudTexture = nullptr;
	Shader*						m_hudShader = nullptr;
	Texture*					m_reticleTexture = nullptr;
	IntVec2						m_reticleSize = IntVec2::ZERO;
	IntVec2						m_spriteSize = IntVec2::ZERO;
	Vec2						m_spritePivot = Vec2::ZERO;

	SpriteAnimDefinition		m_idleAnimation;
	Shader*						m_idleAnimationShader = nullptr;

	SpriteAnimDefinition		m_attackAnimation;
	Shader*						m_attackAnimationShader = nullptr;

	SoundID						m_fireSound = MISSING_SOUND_ID;

	// Gold
	Shader*						m_shader = nullptr;
	bool						m_is3DWeapon = false;
	Model*						m_model = nullptr;
	Texture*					m_texture = nullptr;
	float						m_modelScale = 1.f;

	int							m_particlesOnHit = 100;
	Rgba8						m_hitParticleColor = Rgba8::ORANGE;
	Rgba8						m_fireParticleColor = Rgba8::YELLOW;

	float						m_recoilAngle = 2.f;

public:
	~WeaponDefinition() = default;
	WeaponDefinition() = default;
	WeaponDefinition(XmlElement const* element);

	static void InitializeWeaponDefinitions();

	static std::map<std::string, WeaponDefinition> s_weaponDefs;
};
