#include "Game/Gold/Rock.hpp"

#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"


Rock::Rock(Map* map, Vec3 const& position, EulerAngles const& orientation, float scale, Rgba8 const& tint)
	: StaticActor(map, position)
	, m_tint(tint)
{
	m_transform = Mat44::CreateTranslation3D(m_position);
	m_transform.Append(orientation.GetAsMatrix_iFwd_jLeft_kUp());
	m_transform.AppendScaleUniform3D(scale);

	m_physicsHeight = scale * 0.5f;
	m_physicsRadius = scale * 0.3f;

	std::string modelFileName = "Data/Models/rocka";
	if (g_RNG->RollRandomChance(0.5f))
	{
		modelFileName = "Data/Models/rockb";
	}

	m_model = g_modelLoader->CreateOrGetModelFromObj(modelFileName.c_str(), Mat44(Vec3::SOUTH, Vec3::SKYWARD, Vec3::WEST, Vec3::ZERO));
}

void Rock::Render() const
{
	g_renderer->SetBlendMode(BlendMode::OPAQUE);
	g_renderer->SetDepthMode(DepthMode::ENABLED);
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_BACK);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->BindTexture(nullptr);
	g_renderer->SetModelConstants(m_transform, m_tint);
	g_renderer->DrawIndexBuffer(m_model->GetVertexBuffer(), m_model->GetIndexBuffer(), m_model->GetIndexCount());

	if (m_map->m_game->m_drawDebug)
	{
		RenderDebug();
	}
}
