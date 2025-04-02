#include "Game/Gold/GoldMap.hpp"

#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Player.hpp"
#include "Game/Gold/Tree.hpp"
#include "Game/Gold/Rock.hpp"
#include "Game/Gold/PlayerActor.hpp"
#include "Game/Gold/Dragon.hpp"
#include "Game/WeaponDefinition.hpp"
#include "Game/Weapon.hpp"

#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/VirtualReality/OpenXR.hpp"

#include <string>

GoldMap::~GoldMap()
{
	delete m_fullscreenVBO;
	m_fullscreenVBO = nullptr;
}

GoldMap::GoldMap(Game* game)
{
	m_game = game;
	m_blockModel = g_modelLoader->CreateModelFromObj("Data/Models/block", Mat44(Vec3::SOUTH, Vec3::SKYWARD, Vec3::WEST, Vec3::ZERO));
	m_shader = g_renderer->CreateOrGetShader("Data/Shaders/DiffuseUseShadows", VertexType::VERTEX_PCUTBN);
	m_diffuseShader = g_renderer->CreateOrGetShader("Data/Shaders/Diffuse", VertexType::VERTEX_PCUTBN);
	m_skyboxTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/SpaceSkybox.png");
	m_dimensions = IntVec2(50, 50);

	SpawnInfo playerSpawnInfo;
	playerSpawnInfo.m_actor = "SpawnPoint";
	playerSpawnInfo.m_position = Vec3(10.f, 10.f, 0.f);
	playerSpawnInfo.m_orientation = EulerAngles::ZERO;
	CreateSpawnPoint(playerSpawnInfo);

	PlaceCliffs();
	PlaceTrees();
	PlaceRocks();

	m_renderTargetTexture = g_renderer->CreateRenderTargetTexture("GoldMap::RenderTexture", g_window->GetClientDimensions());

	Vertex_PCU fullscreenQuad[] =
	{
		Vertex_PCU(Vec3(-1.f, -1.f, 0.5f), Rgba8::WHITE, Vec2(0.f, 1.f)),
		Vertex_PCU(Vec3(1.f, -1.f, 0.5f), Rgba8::WHITE, Vec2(1.f, 1.f)),
		Vertex_PCU(Vec3(1.f, 1.f, 0.5f), Rgba8::WHITE, Vec2(1.f, 0.f)),
		/**/
		Vertex_PCU(Vec3(-1.f, -1.f, 0.5f), Rgba8::WHITE, Vec2(0.f, 1.f)),
		Vertex_PCU(Vec3(1.f, 1.f, 0.5f), Rgba8::WHITE, Vec2(1.f, 0.f)),
		Vertex_PCU(Vec3(-1.f, 1.f, 0.5f), Rgba8::WHITE, Vec2(0.f, 0.f))
	};
	m_fullscreenVBO = g_renderer->CreateVertexBuffer(sizeof(fullscreenQuad));
	g_renderer->CopyCPUToGPU(fullscreenQuad, sizeof(fullscreenQuad), m_fullscreenVBO);

	m_shadowShader = g_renderer->CreateOrGetShader("Data/Shaders/ShadowShader", VertexType::VERTEX_PCUTBN);
	m_shadowMap = g_renderer->CreateDepthBuffer("GoldMap::ShadowMap", g_window->GetClientDimensions());

}

void GoldMap::PlaceCliffs()
{
	for (int y = 0; y < 50; y += 4)
	{
		for (int x = 0; x < 50; x += 4)
		{
			if (x == 0 || x == 48)
			{
				float scale = g_RNG->RollRandomFloatInRange(10.f, 15.f);
				float rollDegrees = g_RNG->RollRandomFloatInRange(-10.f, 10.f);
				float pitchDegrees = g_RNG->RollRandomFloatInRange(-10.f, 10.f);
				float yawDegrees = g_RNG->RollRandomFloatInRange(75.f, 105.f);
				unsigned char color = (unsigned char)g_RNG->RollRandomIntInRange(100, 200);
				m_staticActors.push_back(new Rock(this, Vec3((float)x, (float)y, -1.5f), EulerAngles(yawDegrees, pitchDegrees, rollDegrees), scale, Rgba8(color, color, color, 255)));
			}
			else if (y == 0 || y == 48)
			{
				float scale = g_RNG->RollRandomFloatInRange(10.f, 15.f);
				float rollDegrees = g_RNG->RollRandomFloatInRange(-10.f, 10.f);
				float pitchDegrees = g_RNG->RollRandomFloatInRange(-10.f, 10.f);
				float yawDegrees = g_RNG->RollRandomFloatInRange(-15.f, 15.f);
				unsigned char color = (unsigned char)g_RNG->RollRandomIntInRange(100, 200);
				m_staticActors.push_back(new Rock(this, Vec3((float)x, (float)y, -1.5f), EulerAngles(yawDegrees, pitchDegrees, rollDegrees), scale, Rgba8(color, color, color, 255)));
			}
		}
	}
}

void GoldMap::PlaceTrees()
{
	for (int y = 10; y < 40; y++)
	{
		for (int x = 10; x < 40; x++)
		{
			if (g_RNG->RollRandomChance(0.08f))
			{
				if (IsValidSpawnLocation((float)x, (float)y))
				{
					m_staticActors.push_back(new Tree(this, Vec3((float)x, (float)y, 0.f)));
				}
			}
		}
	}
}

void GoldMap::PlaceRocks()
{
	for (int y = 10; y < 40; y++)
	{
		for (int x = 10; x < 40; x++)
		{
			if (g_RNG->RollRandomChance(0.08f))
			{
				if (IsValidSpawnLocation((float)x, (float)y))
				{
					float scale = g_RNG->RollRandomFloatInRange(0.25f, 2.5f);
					//float rollDegrees = g_RNG->RollRandomFloatInRange(-10.f, 10.f);
					//float pitchDegrees = g_RNG->RollRandomFloatInRange(-10.f, 10.f);
					float rollDegrees = 0.f;
					float pitchDegrees = 0.f;
					float yawDegrees = g_RNG->RollRandomFloatInRange(0.f, 360.f);
					unsigned char color = (unsigned char)g_RNG->RollRandomIntInRange(100, 200);
					m_staticActors.push_back(new Rock(this, Vec3((float)x, (float)y, 0.f), EulerAngles(yawDegrees, pitchDegrees, rollDegrees), scale, Rgba8(color, color, color, 255)));
				}
			}
		}
	}
}

void GoldMap::SpawnWave()
{
	for (int enemySoldierIndex = 0; enemySoldierIndex < SOLDIERS_IN_WAVE[m_level]; enemySoldierIndex++)
	{
		SpawnInfo soldierSpawnInfo;
		soldierSpawnInfo.m_actor = m_level == 0 ? "EnemyKnifeSoldier" : "EnemyPistolSoldier";
		soldierSpawnInfo.m_position = g_RNG->RollRandomVec3InAABB3(AABB3(Vec3(10.f, 10.f, 1.f), Vec3((float)m_dimensions.x - 10.f, (float)m_dimensions.y - 10.f, 3.f)));
		soldierSpawnInfo.m_orientation = EulerAngles(g_RNG->RollRandomFloatInRange(-45.f, 45.f), 0.f, 0.f);
		SpawnActor(soldierSpawnInfo);
	}

	for (int ghostIndex = 0; ghostIndex < TANKS_IN_WAVE[m_level]; ghostIndex++)
	{
		SpawnInfo tankSpawnInfo;
		tankSpawnInfo.m_actor = "Tank";
		tankSpawnInfo.m_position = g_RNG->RollRandomVec3InAABB3(AABB3(Vec3(10.f, 10.f, 0.2f), Vec3((float)m_dimensions.x - 10.f, (float)m_dimensions.y - 10.f, 0.5f)));
		tankSpawnInfo.m_orientation = EulerAngles(g_RNG->RollRandomFloatInRange(0.f, 360.f), 0.f, 0.f);
		SpawnActor(tankSpawnInfo);
	}

	m_remainingEnemies += SOLDIERS_IN_WAVE[m_level] + TANKS_IN_WAVE[m_level];
}

void GoldMap::Update()
{
	ShowLevelMessage();
	HandleWaveStart();

	if (m_isCombatMode)
	{
		DebugAddScreenText(Stringf("Enemies Remaining: %d / %d", m_remainingEnemies, (SOLDIERS_IN_WAVE[m_level] + TANKS_IN_WAVE[m_level])), Vec2::ZERO, 25.f, Vec2::ZERO, 0.f, Rgba8::MAROON, Rgba8::RED);
	}

	UpdateActors();
	UpdateVisualActors();
	CollideActors();
	CollideActorsWithStaticActors();
	CollideActorsWithMap();
	UpdateActorPivotPositions();
	
	DeleteDestroyedActors();
}

void GoldMap::UpdateActorPivotPositions()
{
	for (int actorIndex = 0; actorIndex < (int)m_actors.size(); actorIndex++)
	{
		Actor*& actor = m_actors[actorIndex];
		if (actor)
		{
			actor->m_pivotPosition = actor->m_position + actor->GetUpNormal() * actor->m_definition.m_weaponHeight;
		}
	}
}

void GoldMap::Render() const
{
	// Render pass
	g_renderer->ClearRTV(Rgba8::BLACK, m_renderTargetTexture);

	std::vector<Vertex_PCU> skyboxVerts;
	AddVertsForAABB3(skyboxVerts, AABB3(Vec3(-150.f, -150.f, -100.f), Vec3(150.f, 150.f, 100.f)), Rgba8::DEEP_SKY_BLUE, AABB2(Vec2::ZERO, Vec2(1.f, 1.f)));
	g_renderer->SetBlendMode(BlendMode::ALPHA);
	g_renderer->SetDepthMode(DepthMode::ENABLED);
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_FRONT);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->BindShader(nullptr);
	g_renderer->BindTexture(nullptr);
	g_renderer->DrawVertexArray(skyboxVerts);

	g_renderer->BindShader(m_shader);
	g_renderer->BindDepthBuffer(m_shadowMap);
	RenderScene();

	g_renderer->SetBlendMode(BlendMode::ALPHA);
	g_renderer->SetDepthMode(DepthMode::ENABLED);
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_BACK);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->BindShader(m_diffuseShader);
	g_renderer->BindTexture(nullptr);
	RenderVisualActors();

	g_renderer->BindDepthBuffer(nullptr);
}

void GoldMap::RenderScreen() const
{
	// Render screen
	g_renderer->SetDepthMode(DepthMode::DISABLED);
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_BACK);
	g_renderer->SetModelConstants();
	g_renderer->BindShader(nullptr);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->SetBlendMode(BlendMode::ALPHA);
	g_renderer->BindTexture(nullptr);
	m_game->m_player->RenderScreen();
}

void GoldMap::RenderCustomScreens() const
{
	// Shadow Pass
	g_renderer->BeginCamera(g_app->m_worldCamera);
	g_renderer->ClearDSV(m_shadowMap);
	g_renderer->SetBlendMode(BlendMode::ALPHA);
	g_renderer->SetDepthMode(DepthMode::ENABLED);
	g_renderer->SetLightConstants(m_game->m_sunDirection.GetNormalized(), m_game->m_sunIntensity, m_game->m_ambientIntensity);
	g_renderer->SetModelConstants();
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_BACK);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->BindShader(m_shadowShader);
	g_renderer->BindTexture(nullptr);
	g_renderer->SetDSV(m_shadowMap);
	RenderStaticActors();
	g_renderer->EndCamera(g_app->m_worldCamera);
	// End Shadow pass
}

void GoldMap::RenderScene() const
{
	g_renderer->SetBlendMode(BlendMode::OPAQUE);
	g_renderer->SetDepthMode(DepthMode::ENABLED);
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_BACK);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->SetLightConstants(m_game->m_sunDirection.GetNormalized(), m_game->m_sunIntensity, m_game->m_ambientIntensity);
	//g_theRenderer->BindShader(m_shader);
	g_renderer->BindTexture(nullptr);

	for (int y = 0; y < 50; y++)
	{
		for (int x = 0; x < 50; x++)
		{
			g_renderer->SetModelConstants(Mat44::CreateTranslation3D(Vec3((float)x, (float)y, -1.f)), Rgba8::WHITE);
			g_renderer->DrawIndexBuffer(m_blockModel->GetVertexBuffer(), m_blockModel->GetIndexBuffer(), m_blockModel->GetIndexCount());
		}
	}

	RenderStaticActors();
	RenderActors();
}

void GoldMap::RenderStaticActors() const
{
	for (int actorIndex = 0; actorIndex < (int)m_staticActors.size(); actorIndex++)
	{
		m_staticActors[actorIndex]->Render();
	}
}

void GoldMap::RenderVisualActors() const
{
	for (int actorIndex = 0; actorIndex < (int)m_visualActors.size(); actorIndex++)
	{
		if (m_visualActors[actorIndex])
		{
			m_visualActors[actorIndex]->Render();
		}
	}
}

void GoldMap::UpdateVisualActors()
{
	for (int actorIndex = 0; actorIndex < (int)m_visualActors.size(); actorIndex++)
	{
		if (m_visualActors[actorIndex])
		{
			m_visualActors[actorIndex]->Update();
		}
	}
}

DoomRaycastResult GoldMap::RaycastVsWalls(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDistance) const
{
	DoomRaycastResult result;
	result.m_impactDistance = maxDistance;
	result.m_rayStartPosition = startPos;
	result.m_rayForwardNormal = fwdNormal;
	result.m_rayMaxLength = maxDistance;
	result.m_didImpact = false;

	DoomRaycastResult raycastVsStaticActorsResult;
	raycastVsStaticActorsResult.m_impactDistance = maxDistance;
	raycastVsStaticActorsResult.m_rayStartPosition = startPos;
	raycastVsStaticActorsResult.m_rayForwardNormal = fwdNormal;
	raycastVsStaticActorsResult.m_rayMaxLength = maxDistance;
	raycastVsStaticActorsResult.m_didImpact = false;

	for (int staticActorIndex = 0; staticActorIndex < (int)m_staticActors.size(); staticActorIndex++)
	{
		StaticActor* const& staticActor = m_staticActors[staticActorIndex];

		RaycastResult3D raycastVsActorResult = RaycastVsCylinder3D(startPos, fwdNormal, maxDistance, staticActor->m_position, staticActor->m_position + Vec3::SKYWARD * staticActor->m_physicsHeight, staticActor->m_physicsRadius);
		if (raycastVsActorResult.m_didImpact && raycastVsActorResult.m_impactDistance < raycastVsStaticActorsResult.m_impactDistance)
		{
			raycastVsStaticActorsResult = raycastVsActorResult;
		}
	}

	if (raycastVsStaticActorsResult.m_didImpact)
	{
		result.m_didImpact = true;
		result.m_impactDistance = raycastVsStaticActorsResult.m_impactDistance;
		result.m_impactNormal = raycastVsStaticActorsResult.m_impactNormal;
	}

	if (fwdNormal.z < 0.f)
	{
		float rayFloorImpactDistance = -startPos.z / fwdNormal.z;
		if (rayFloorImpactDistance < 0.f)
		{
			rayFloorImpactDistance = FLT_MAX;
		}
		if (rayFloorImpactDistance < result.m_impactDistance)
		{
			result.m_didImpact = true;
			result.m_impactDistance = rayFloorImpactDistance;
			result.m_impactNormal = Vec3::SKYWARD;
		}
	}

	result.m_impactPosition = startPos + fwdNormal * result.m_impactDistance;

	return result;
}

Player* GoldMap::GetCurrentRenderingPlayer() const
{
	return m_game->m_player;
}

void GoldMap::CollideActorsWithStaticActors()
{
	for (int actorIndex = 0; actorIndex < (int)m_actors.size(); actorIndex++)
	{
		Actor*& actor = m_actors[actorIndex];
		if (!actor)
		{
			continue;
		}

		for (int staticActorIndex = 0; staticActorIndex < (int)m_staticActors.size(); staticActorIndex++)
		{
			StaticActor*& staticActor = m_staticActors[staticActorIndex];

			if (!staticActor)
			{
				continue;
			}

			if (DoZCylindersOverlap(actor->m_position, actor->m_position + Vec3::SKYWARD * actor->m_physicsHeight, actor->m_physicsRadius, staticActor->m_position, staticActor->m_position + Vec3::SKYWARD * staticActor->m_physicsHeight, staticActor->m_physicsRadius))
			{
				actor->OnCollideWithStatic(staticActor);
			}
		}
	}
}

void GoldMap::CollideActorWithFloorAndCeiling(Actor* actor)
{
	if (actor->m_position.z < 0.f)
	{
		if (IsPointInsideAABB2(actor->m_position.GetXY(), AABB2(Vec2::ZERO, m_dimensions.GetAsVec2())))
		{
			actor->m_position.z = 0.f;
			actor->OnCollide(nullptr);
		}
		else
		{
			actor->Die();
		}
	}
}

bool GoldMap::IsValidSpawnLocation(float x, float y) const
{
	for (int staticActorIndex = 0; staticActorIndex < (int)m_staticActors.size(); staticActorIndex++)
	{
		StaticActor* const& staticActor = m_staticActors[staticActorIndex];
		if (IsPointInsideDisc2D(Vec2(x, y), staticActor->m_position.GetXY(), staticActor->m_physicsRadius))
		{
			return false;
		}
	}

	return true;
}

void GoldMap::DeleteDestroyedActors()
{
	for (int actorIndex = 0; actorIndex < (int)m_actors.size(); actorIndex++)
	{
		Actor*& actor = m_actors[actorIndex];
		if (actor && actor->m_isDestroyed)
		{
			if (actor->m_definition.m_faction == Faction::DEMON)
			{
				m_remainingEnemies--;
				if (m_remainingEnemies == 0)
				{
					IncrementLevel();
				}
			}

			delete actor;
			actor = nullptr;
		}
	}

	for (int actorIndex = 0; actorIndex < (int)m_visualActors.size(); actorIndex++)
	{
		Actor*& actor = m_visualActors[actorIndex];
		if (actor && actor->m_isDestroyed)
		{
			delete actor;
			actor = nullptr;
		}
	}
}

void GoldMap::ShowLevelMessage()
{
	if (m_isCombatMode)
	{
		return;
	}

	switch(m_level)
	{
		case 0:
		{
			DebugAddScreenText("You ready? Hit R to start wave!", Vec2(g_screenSizeX, 0.f), 20.f, Vec2(1.f, 0.f), 0.f);
			break;
		}
		case 1:
		{
			DebugAddScreenText("That was easy, wasn't it? Let's give them guns now! R when you're ready.", Vec2(g_screenSizeX, 0.f), 20.f, Vec2(1.f, 0.f), 0.f);
			break;
		}
		case 2:
		{
			DebugAddScreenText("Alright, time to prove yourself as a hardcore gamer now! R when you're ready", Vec2(g_screenSizeX, 0.f), 20.f, Vec2(1.f, 0.f), 0.f);
			break;
		}
		case 3:
		{
			DebugAddScreenText("That. Was. Awesome! Hop around this cool map or hit escape to return to the Attract screen.", Vec2(g_screenSizeX, 0.f), 20.f, Vec2(1.f, 0.f), 0.f);
			break;
		}
	}
}

void GoldMap::IncrementLevel()
{
	m_level++;
	m_isCombatMode = false;

	switch (m_level)
	{
		case 0:
		{
			if (g_input->WasKeyJustPressed('R'))
			{
				SpawnWave();
				m_isCombatMode = true;
			}
			break;
		}
		case 1:
		{
			m_game->m_player->GetActor()->m_weapons.push_back(new Weapon(WeaponDefinition::s_weaponDefs["AK3D"]));
			break;
		}
		case 2:
		{
			break;
		}
	}
}

void GoldMap::HandleWaveStart()
{
	if (m_isCombatMode)
	{
		return;
	}

	if (g_input->WasKeyJustPressed('R'))
	{
		SpawnWave();
		m_isCombatMode = true;
	}

	if (g_openXR && g_openXR->IsInitialized())
	{
		VRController leftController = g_openXR->GetLeftController();
		if (leftController.WasSelectButtonJustPressed())
		{
			SpawnWave();
			m_isCombatMode = true;
		}
	}
}
