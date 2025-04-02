#pragma once

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Models/Model.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/AnimationGroupDefinition.hpp"
#include "Engine/Renderer/Spritesheet.hpp"
#include "Engine/Renderer/Texture.hpp"

#include <map>
#include <string>
#include <vector>

class Weapon;

enum class Faction
{
	INVALID,

	SPAWNPOINT,
	MARINE,
	DEMON,
	NEUTRAL,

	COUNT
};

Faction GetFactionFromString(std::string factionString);
BillboardType GetBillboardTypeFromString(std::string billboardType);

struct ActorDefinition
{
public:
	std::string					m_name = "";
	Faction						m_faction = Faction::INVALID;
	float						m_health = 100.f;
	bool						m_canBePossessed = false;
	bool						m_dieOnSpawn = false;
	float						m_corpseLifetime = 0.f;
	bool						m_visible = false;
	Rgba8						m_solidColor = Rgba8::WHITE;
	Rgba8						m_wireframeColor = Rgba8::WHITE;
	float						m_physicsRadius = 0.f;
	float						m_physicsHeight = 0.f;
	bool						m_collidesWithWorld = false;
	bool						m_collidesWithActors = false;
	bool						m_simulated = false;
	float						m_walkSpeed = 4.f;
	float						m_runSpeed = 10.f;
	float						m_turnSpeed = 90.f;
	float						m_drag = 3.f;
	float						m_eyeHeight = 0.f;
	float						m_eyeFov = 60.f;
	std::vector<Weapon*>		m_weapons;
	float						m_weaponHeight = 0.f;
	bool						m_dieOnCollide = false;
	FloatRange					m_damageOnCollide = FloatRange::ZERO;
	float						m_impulseOnCollide = 0.f;
	bool						m_aiEnabled = false;
	float						m_sightRadius = 0.f;
	float						m_sightAngle = 0.f;
	bool						m_isLit = false;
	bool						m_isRounded = false;
	Vec2						m_size = Vec2::ZERO;
	Vec2						m_pivot = Vec2::ZERO;
	BillboardType				m_billboardType = BillboardType::NONE;
	Shader*						m_shader = nullptr;
	Texture*					m_texture = nullptr;
	SpriteSheet*				m_spriteSheet = nullptr;
	IntVec2						m_spriteSheetCellCount = IntVec2::ZERO;
	std::vector<AnimationGroupDefinition> m_animations;
	SoundID						m_hurtSound = MISSING_SOUND_ID;
	SoundID						m_deathSound = MISSING_SOUND_ID;
	SoundID						m_seeSound = MISSING_SOUND_ID;

	// Gold
	float						m_gravityScale = 0.f;
	bool						m_is3DActor = false;
	Model*						m_model = nullptr;
	float						m_modelScale = 0.5f;
	BlendMode					m_blendMode = BlendMode::OPAQUE;

	bool						m_explodeOnDie = false;
	float						m_explosionRadius = 0.f;
	int							m_explosionParticles = 0;
	Rgba8						m_explosionParticleColor = Rgba8::WHITE;
	FloatRange					m_explosionParticleSize = FloatRange::ZERO;
	float						m_explosionParticleLifetime = 0.f;
	FloatRange					m_explosionDamage = FloatRange::ZERO;
	float						m_explosionParticleSpeed = 0.f;
	float						m_impulseOnExplode = 0.f;

	bool						m_showVisualParticles = false;
	int							m_visualParticles = 0;
	FloatRange					m_visualParticleSize = FloatRange::ZERO;
	float						m_visualParticleLifetime = 0.f;
	float						m_visualParticleSpeed = 0.f;
	Rgba8						m_visualParticleColor = Rgba8::WHITE;

public:
	~ActorDefinition() = default;
	ActorDefinition() = default;
	ActorDefinition(XmlElement const* element);
	AnimationGroupDefinition GetAnimationGroupByName(std::string animationGroupName) const;

	static void					InitializeActorDefinitions();

	static std::map<std::string, ActorDefinition> s_actorDefs;
};
