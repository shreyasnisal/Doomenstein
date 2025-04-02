#pragma once

#include "Engine/Core/Clock.hpp"

#include "Game/WeaponDefinition.hpp"
#include "Game/ActorUID.hpp"

class Actor;
class Map;

class Weapon
{
public:
	~Weapon() = default;
	Weapon() = default;
	Weapon(Weapon const& copyFrom) = default;
	explicit Weapon(WeaponDefinition const& definition, XRHand equipHand = XRHand::NONE);
	
	void Update();
	void Render() const;

	void OnEquipped(Actor* owner);
	void Fire();
	Vec3 const GetRandomFireDirection(Actor const* actor, float deviationAngle) const;
	float GetRange() const;

public:
	Map* m_map = nullptr;
	ActorUID m_ownerUID = ActorUID::INVALID;
	Stopwatch m_refireTimer;
	WeaponDefinition m_definition;

	SpriteAnimDefinition m_currentAnimation;
	Clock* m_animationClock = nullptr;
	Shader* m_currentShader = nullptr;

	std::vector<Vertex_PCU> m_reticleVertexes;
	SoundID m_fireSoundPlayback = MISSING_SOUND_ID;

	Mat44 m_transform;

	XRHand m_equipHand = XRHand::NONE;
};
