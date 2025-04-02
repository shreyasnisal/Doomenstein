#include "Game/Gold/StaticActor.hpp"

#include "Engine/Renderer/DebugRenderSystem.hpp"

StaticActor::StaticActor(Map* map, Vec3 const& position)
	: m_map(map)
	, m_position(position)
{
}

void StaticActor::RenderDebug() const
{
	DebugAddWorldWireCylinder(m_position, m_position + Vec3::SKYWARD * m_physicsHeight, m_physicsRadius, 0.f, Rgba8::MAGENTA, Rgba8::MAGENTA);	
}
