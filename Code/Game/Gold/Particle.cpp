#include "Game/Gold/Particle.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/Game.hpp"

#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"


Particle::~Particle()
{
	delete m_vertexBuffer;
	m_vertexBuffer = nullptr;

	delete m_indexBuffer;
	m_indexBuffer = nullptr;
}

Particle::Particle(Map* map, SpawnInfo spawnInfo, float radius, Rgba8 const& color, float lifetime)
	: Actor(map, spawnInfo, ActorUID::INVALID)
	, m_size(radius)
{
	m_color = color;

	std::vector<Vertex_PCUTBN> vertexes;
	std::vector<unsigned int> indexes;
	AddVertsForAABB3(vertexes, indexes, AABB3(Vec3(-1.f, -1.f, -1.f) * m_size, Vec3(1.f, 1.f, 1.f) * m_size), Rgba8::WHITE);

	m_vertexBuffer = g_renderer->CreateVertexBuffer(vertexes.size() * sizeof(Vertex_PCUTBN), VertexType::VERTEX_PCUTBN);
	m_indexBuffer = g_renderer->CreateIndexBuffer(indexes.size() * sizeof(unsigned int));
	g_renderer->CopyCPUToGPU(vertexes.data(), vertexes.size() * sizeof(Vertex_PCUTBN), m_vertexBuffer);
	g_renderer->CopyCPUToGPU(indexes.data(), indexes.size() * sizeof(unsigned int), m_indexBuffer);

	m_definition.m_is3DActor = true;
	m_definition.m_corpseLifetime = lifetime;
	Die();
}

void Particle::Update()
{
	float deltaSeconds = m_map->m_game->m_gameClock.GetDeltaSeconds();
	m_velocity += m_acceleration * deltaSeconds;
	m_position += m_velocity * deltaSeconds;
	m_acceleration = Vec3::ZERO;

	if (m_lifetimeTimer.HasDurationElapsed())
	{
		m_isDead = true;
		m_isDestroyed = true;
	}
}

void Particle::Render() const
{
	g_renderer->SetBlendMode(BlendMode::ADDITIVE);
	g_renderer->SetDepthMode(DepthMode::ENABLED);
	float colorInterpolationParametric = EaseOutQuadratic(m_lifetimeTimer.GetElapsedFraction());
	Rgba8 color = Interpolate(m_color, Rgba8(m_color.r, m_color.g, m_color.b, 0), colorInterpolationParametric);
	g_renderer->SetModelConstants(Mat44::CreateTranslation3D(m_position), color);
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_BACK);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->BindTexture(nullptr);
	g_renderer->DrawIndexBuffer(m_vertexBuffer, m_indexBuffer, (int)(m_indexBuffer->m_size / sizeof(unsigned int)));
}
