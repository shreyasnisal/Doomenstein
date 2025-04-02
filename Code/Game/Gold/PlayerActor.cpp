#include "Game/Gold/PlayerActor.hpp"


PlayerActor::PlayerActor(Map* map, ActorUID const& uid, Vec3 const& position, EulerAngles const& orientation)
{
	m_UID = uid;
	m_map = map;
	m_position = position;
	m_orientation = orientation;

	m_physicsHeight = 1.f;
	m_physicsRadius = 0.25f;

	m_definition.m_gravityScale = 1.f;
}

Vec3 const PlayerActor::GetEyePosition() const
{
	return m_position + Vec3::SKYWARD * m_physicsHeight;
}

void PlayerActor::Render() const
{
}
