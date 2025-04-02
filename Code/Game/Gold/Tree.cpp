#include "Game/Gold/Tree.hpp"

#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/StringUtils.hpp"


Tree::Tree(Map* map, Vec3 const& position, float scale)
	: StaticActor(map, position)
{
	int treeModelIndex = g_RNG->RollRandomIntInRange(0, 2);
	std::string modelFileName;
	switch (treeModelIndex)
	{
		case 0:
		{
			modelFileName = "Data/Models/tree";
			break;
		}
		case 1:
		{
			modelFileName = "Data/Models/treePine";
			break;
		}
		case 2:
		{
			modelFileName = "Data/Models/treePineSmall";
			break;
		}
	}

	m_physicsHeight = scale;
	m_physicsRadius = scale * 0.2f;

	m_model = g_modelLoader->CreateOrGetModelFromObj(modelFileName.c_str(), Mat44(Vec3::SOUTH, Vec3::SKYWARD, Vec3::WEST, Vec3::ZERO));
}

void Tree::Render() const
{
	g_renderer->SetBlendMode(BlendMode::OPAQUE);
	g_renderer->SetDepthMode(DepthMode::ENABLED);
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_BACK);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->BindTexture(nullptr);
	g_renderer->SetModelConstants(Mat44::CreateTranslation3D(m_position));
	g_renderer->DrawIndexBuffer(m_model->GetVertexBuffer(), m_model->GetIndexBuffer(), m_model->GetIndexCount());


	if (m_map->m_game->m_drawDebug)
	{
		RenderDebug();
	}
}

