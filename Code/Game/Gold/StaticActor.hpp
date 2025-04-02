#pragma once

#include "Engine/Math/Vec3.hpp"

#include "Game/Map.hpp"

class StaticActor
{
public:
	virtual ~StaticActor() = default;
	StaticActor(Map* map, Vec3 const& position);

	virtual void Render() const = 0;
	virtual void RenderDebug() const;

public:
	Map* m_map;
	Vec3 m_position;
	float m_physicsHeight = 0.f;
	float m_physicsRadius = 0.f;
	bool isOptional = false;
};
