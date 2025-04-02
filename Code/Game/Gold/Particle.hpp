#pragma once

#include "Game/Actor.hpp"

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"

class Map;

class Particle : public Actor
{
public:
	~Particle();
	Particle() = default;
	Particle(Map* map, SpawnInfo spawnInfo, float radius, Rgba8 const& color, float lifetime);

	virtual void Update() override;
	virtual void Render() const override;

public:
	Rgba8 m_color;
	float m_size = 0.f;

	VertexBuffer* m_vertexBuffer = nullptr;
	IndexBuffer* m_indexBuffer = nullptr;
};

