#include "Game/Map.hpp"

#include "Game/Actor.hpp"
#include "Game/AI.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/Player.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/Gold/Particle.hpp"

#include "Engine/Core/Image.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"

Map::~Map()
{
	delete m_tileVertexBuffer;
	m_tileVertexBuffer = nullptr;

	delete m_tileIndexBuffer;
	m_tileIndexBuffer = nullptr;
}

Map::Map(Game* game, MapDefinition mapDef)
	: m_game(game)
	, m_definition(mapDef)
{

	if (!m_definition.m_imagePath.empty())
	{
		ConstructMapFromImage();
	}
	else
	{
		ERROR_AND_DIE("No image provided in map definition, could not generate map!");
	}

	CreateTileHeatMaps();

	for (int spawnIndex = 0; spawnIndex < (int)m_definition.m_spawnInfos.size(); spawnIndex++)
	{
		SpawnInfo& spawnInfo = m_definition.m_spawnInfos[spawnIndex];
		if (GetFactionFromString(spawnInfo.m_actor) == Faction::DEMON)
		{
			SpawnActor(spawnInfo);
		}
		else if (GetFactionFromString(spawnInfo.m_actor) == Faction::SPAWNPOINT)
		{
			CreateSpawnPoint(spawnInfo);
		}
	}
}

void Map::ConstructMapFromImage()
{
	Image mapImage = Image(m_definition.m_imagePath.c_str());
	m_definition.m_dimensions = mapImage.GetDimensions();

	int numTiles = m_definition.m_dimensions.x * m_definition.m_dimensions.y;
	m_tiles.resize(numTiles);

	for (int tileY = 0; tileY < GetDimensions().y; tileY++)
	{
		for (int tileX = 0; tileX < GetDimensions().x; tileX++)
		{
			Rgba8 mapImageTexelColor = mapImage.GetTexelColor(IntVec2(tileX, tileY));

			bool wasMatchingTileFound = false;
			for (auto tileDefIter = TileDefinition::s_tileDefs.begin(); tileDefIter != TileDefinition::s_tileDefs.end(); tileDefIter++) {
				if (tileDefIter->second.m_mapImageColor.r == mapImageTexelColor.r && tileDefIter->second.m_mapImageColor.g == mapImageTexelColor.g && tileDefIter->second.m_mapImageColor.b == mapImageTexelColor.b)
				{
					wasMatchingTileFound = true;
					if (g_RNG->RollRandomIntInRange(0, 254) < static_cast<int>(mapImageTexelColor.a))
					{
						SetTileType(IntVec2(tileX, tileY), tileDefIter->first);
					}
				}
			}
			if (!wasMatchingTileFound)
			{
				ERROR_AND_DIE(Stringf("Could not find Tile Definition matching image texel color at (%d, %d)", tileX, tileY));
			}
		}
	}

	InitializeTiles();
}

void Map::InitializeTiles()
{
	int estimatedVertexCount = 3 * 2 * m_definition.m_dimensions.x * m_definition.m_dimensions.y;
	std::vector<Vertex_PCUTBN> tileVertexes;
	std::vector<unsigned int> tileIndexes;
	tileVertexes.reserve(estimatedVertexCount);
	tileIndexes.reserve(estimatedVertexCount);

	for (int tileIndex = 0; tileIndex < static_cast<int>(m_tiles.size()); tileIndex++)
	{
		if (m_tiles[tileIndex].IsSolid())
		{
			AABB2 tileWallUVs = m_definition.m_terrainSpriteSheet->GetSpriteUVs(m_tiles[tileIndex].GetWallSpriteCoords().y * m_definition.m_spriteSheetDimensions.x + m_tiles[tileIndex].GetWallSpriteCoords().x);
			AddVertsForWall(tileVertexes, tileIndexes, tileIndex, tileWallUVs);
		}
		else
		{
			AABB2 tileFloorUVs = m_definition.m_terrainSpriteSheet->GetSpriteUVs(m_tiles[tileIndex].GetFloorSpriteCoords().y * m_definition.m_spriteSheetDimensions.x + m_tiles[tileIndex].GetFloorSpriteCoords().x);
			AABB2 tileCeilingUVs = m_definition.m_terrainSpriteSheet->GetSpriteUVs(m_tiles[tileIndex].GetCeilingSpriteCoords().y * m_definition.m_spriteSheetDimensions.x + m_tiles[tileIndex].GetCeilingSpriteCoords().x);
			AddVertsForTile(tileVertexes, tileIndexes, tileIndex, tileFloorUVs, tileCeilingUVs);
		}
	}

	m_tileVertexBuffer = g_renderer->CreateVertexBuffer(tileVertexes.size() * sizeof(Vertex_PCUTBN), VertexType::VERTEX_PCUTBN);
	m_tileIndexBuffer = g_renderer->CreateIndexBuffer(tileIndexes.size() * sizeof(unsigned int));
	g_renderer->CopyCPUToGPU(reinterpret_cast<void*>(tileVertexes.data()), tileVertexes.size() * sizeof(Vertex_PCUTBN), m_tileVertexBuffer);
	g_renderer->CopyCPUToGPU(reinterpret_cast<void*>(tileIndexes.data()), tileIndexes.size() * sizeof(unsigned int), m_tileIndexBuffer);
}

void Map::CreateTileHeatMaps()
{
	m_solidMap = new TileHeatMap(m_definition.m_dimensions);
	std::vector<float> heatMapValues(m_definition.m_dimensions.x * m_definition.m_dimensions.y);
	
	for (int tileIndex = 0; tileIndex < m_definition.m_dimensions.x * m_definition.m_dimensions.y; tileIndex++)
	{
		if (m_tiles[tileIndex].IsSolid())
		{
			heatMapValues[tileIndex] = 0.f;
		}
		else
		{
			heatMapValues[tileIndex] = 1.f;
		}
	}
	m_solidMap->SetAllValues(heatMapValues);
}

void Map::RenderTiles() const
{
	g_renderer->SetBlendMode(BlendMode::OPAQUE);
	g_renderer->SetDepthMode(DepthMode::ENABLED);
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_BACK);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->SetModelConstants();
	g_renderer->SetLightConstants(m_game->m_sunDirection.GetNormalized(), m_game->m_sunIntensity, m_game->m_ambientIntensity);
	g_renderer->BindShader(m_definition.m_shader);
	g_renderer->BindTexture(m_definition.m_terrainSpriteSheet->GetTexture());
	g_renderer->DrawIndexBuffer(m_tileVertexBuffer, m_tileIndexBuffer, (int)(m_tileIndexBuffer->m_size / sizeof(unsigned int)));
}

void Map::AddVertsForTile(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, int tileIndex, AABB2 const& floorUVs, AABB2 const& ceilingUVs) const
{
	Tile tile = m_tiles[tileIndex];
	AABB2 bounds = tile.GetBounds();
	Rgba8 color = tile.GetColor();
	Vec3 BL = bounds.m_mins.ToVec3();
	Vec3 BR = Vec3(bounds.m_maxs.x, bounds.m_mins.y, 0.f);
	Vec3 TR = bounds.m_maxs.ToVec3();
	Vec3 TL = Vec3(bounds.m_mins.x, bounds.m_maxs.y, 0.f);
	AddVertsForQuad3D(verts, indexes, BL, BR, TR, TL , color, AABB2(floorUVs.m_mins, floorUVs.m_maxs));

	Vec3 ceilingBL = Vec3(BL.x, BL.y, 1.f);
	Vec3 ceilingBR = Vec3(BR.x, BR.y, 1.f);
	Vec3 ceilingTR = Vec3(TR.x, TR.y, 1.f);
	Vec3 ceilingTL = Vec3(TL.x, TL.y, 1.f);
	AddVertsForQuad3D(verts, indexes, ceilingTR, ceilingBR, ceilingBL, ceilingTL, color, AABB2(ceilingUVs.m_mins, ceilingUVs.m_maxs));
}

void Map::AddVertsForWall(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, int tileIndex, AABB2 const& wallUVs) const
{
	Tile tile = m_tiles[tileIndex];
	AABB2 bounds = tile.GetBounds();
	Rgba8 color = tile.GetColor();
	Vec3 wallMins = Vec3(bounds.m_mins.x, bounds.m_mins.y, 0.f);
	Vec3 wallMaxs = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, 1.f);
	Vec3 BLF = Vec3(wallMins.x, wallMaxs.y, wallMins.z);
	Vec3 BRF = Vec3(wallMins.x, wallMins.y, wallMins.z);
	Vec3 TRF = Vec3(wallMins.x, wallMins.y, wallMaxs.z);
	Vec3 TLF = Vec3(wallMins.x, wallMaxs.y, wallMaxs.z);
	Vec3 BLB = Vec3(wallMaxs.x, wallMaxs.y, wallMins.z);
	Vec3 BRB = Vec3(wallMaxs.x, wallMins.y, wallMins.z);
	Vec3 TRB = Vec3(wallMaxs.x, wallMins.y, wallMaxs.z);
	Vec3 TLB = Vec3(wallMaxs.x, wallMaxs.y, wallMaxs.z);

	AddVertsForQuad3D(verts, indexes, BRB, BLB, TLB, TRB, color, wallUVs); // +X
	AddVertsForQuad3D(verts, indexes, BLF, BRF, TRF, TLF, color, wallUVs); // -X
	AddVertsForQuad3D(verts, indexes, BLB, BLF, TLF, TLB, color, wallUVs); // +Y
	AddVertsForQuad3D(verts, indexes, BRF, BRB, TRB, TRF, color, wallUVs); // -Y
}

void Map::UpdateActors()
{
	for (int actorIndex = 0; actorIndex < (int)m_actors.size(); actorIndex++)
	{
		Actor*& actor = m_actors[actorIndex];
		if (actor)
		{
			actor->Update();
		}
	}
}

void Map::Update()
{
	UpdateActors();
	CollideActors();
	CollideActorsWithMap();

	DeleteDestroyedActors();
}

void Map::Render() const
{
}

void Map::RenderActors() const
{
	for (int actorIndex = 0; actorIndex < (int)m_actors.size(); actorIndex++)
	{
		Actor* const& actor = m_actors[actorIndex];
		if (actor)
		{
			actor->Render();
		}
	}
}

void Map::SetTileType(IntVec2 const& tileCoords, std::string tileTypeName)
{
	int tileIndex = tileCoords.x + tileCoords.y * GetDimensions().x;
	m_tiles[tileIndex] = Tile(tileTypeName, tileCoords.x, tileCoords.y);
}

DoomRaycastResult Map::RaycastVsAll(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDistance, Actor* actorToExclude) const
{
	DoomRaycastResult raycastVsActorsResult = RaycastVsActors(startPos, fwdNormal, maxDistance, actorToExclude);
	DoomRaycastResult raycastVsWallsResult = RaycastVsWalls(startPos, fwdNormal, maxDistance);

	if (raycastVsActorsResult.m_didImpact && raycastVsWallsResult.m_didImpact)
	{
		if (raycastVsActorsResult.m_impactDistance < raycastVsWallsResult.m_impactDistance)
		{
			return raycastVsActorsResult;
		}
		else
		{
			return raycastVsWallsResult;
		}
	}
	else if (raycastVsActorsResult.m_didImpact)
	{
		return raycastVsActorsResult;
	}
	else if (raycastVsWallsResult.m_didImpact)
	{
		return raycastVsWallsResult;
	}
	else
	{
		return DoomRaycastResult();
	}
}

DoomRaycastResult Map::RaycastVsActors(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDistance, Actor* actorToExclude) const
{
	DoomRaycastResult result;
	result.m_impactDistance = maxDistance;
	result.m_rayStartPosition = startPos;
	result.m_rayForwardNormal = fwdNormal;
	result.m_rayMaxLength = maxDistance;
	result.m_didImpact = false;

	for (int actorIndex = 0; actorIndex < (int)m_actors.size(); actorIndex++)
	{
		if (!(m_actors[actorIndex] == actorToExclude) && IsActorAlive(m_actors[actorIndex]))
		{
			Actor* const& actor = m_actors[actorIndex];

			RaycastResult3D raycastVsActorResult = RaycastVsCylinder3D(startPos, fwdNormal, maxDistance, actor->m_position, actor->m_position + actor->GetUpNormal() * actor->m_physicsHeight, actor->m_physicsRadius);
			if (raycastVsActorResult.m_didImpact && raycastVsActorResult.m_impactDistance < result.m_impactDistance)
			{
				result = raycastVsActorResult;
				result.m_impactActorUID = actor->m_UID;
			}
		}
	}

	return result;
}

DoomRaycastResult Map::RaycastVsWalls(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDistance) const
{
	DoomRaycastResult result;
	result.m_didImpact = false;
	result.m_impactDistance = maxDistance;
	result.m_rayStartPosition = startPos;
	result.m_rayForwardNormal = fwdNormal;
	result.m_rayMaxLength = maxDistance;

	RaycastResult3D raycastVsTilesResult = m_solidMap->Raycast(startPos, fwdNormal, maxDistance, 0.f, 1.f);
	if (raycastVsTilesResult.m_didImpact)
	{
		result.m_didImpact = true;
		result.m_impactDistance = raycastVsTilesResult.m_impactDistance;
		result.m_impactNormal = raycastVsTilesResult.m_impactNormal;
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

	if (fwdNormal.z > 0.f)
	{
		float rayCeilingImpactDistance = (1.f - startPos.z) / fwdNormal.z;
		if (rayCeilingImpactDistance < 0.f)
		{
			rayCeilingImpactDistance = FLT_MAX;
		}
		if (rayCeilingImpactDistance < result.m_impactDistance)
		{
			result.m_didImpact = true;
			result.m_impactDistance = rayCeilingImpactDistance;
			result.m_impactNormal = Vec3::GROUNDWARD;
		}
	}

	result.m_impactPosition = startPos + fwdNormal * result.m_impactDistance;

	return result;
}

void Map::CollideActors()
{
	for (int actorIndex = 0; actorIndex < (int)m_actors.size(); actorIndex++)
	{
		if (!IsActorAlive(m_actors[actorIndex]))
		{
			continue;
		}

		for (int otherActorIndex = actorIndex + 1; otherActorIndex < (int)m_actors.size(); otherActorIndex++)
		{
			if (IsActorAlive(m_actors[otherActorIndex]))
			{
				CollideActors(m_actors[actorIndex], m_actors[otherActorIndex]);
			}
		}
	}
}

void Map::CollideActors(Actor* actorA, Actor* actorB)
{
	if (DoZCylindersOverlap(actorA->m_position, actorA->m_position + Vec3::SKYWARD * actorA->m_physicsHeight, actorA->m_physicsRadius, actorB->m_position, actorB->m_position + Vec3::SKYWARD * actorB->m_physicsHeight, actorB->m_physicsRadius))
	{
		if (IsOwner(actorA, actorB) || IsOwner(actorB, actorA))
		{
			return;
		}

		if (actorA->m_ownerUID == actorB->m_ownerUID && actorA->m_ownerUID != ActorUID::INVALID)
		{
			return;
		}

		if (actorA->m_ownerUID != ActorUID::INVALID && actorB->m_ownerUID != ActorUID::INVALID)
		{
			return;
		}

		actorA->OnCollide(actorB);
		actorB->OnCollide(actorA);
	}
}

void Map::CollideActorsWithMap()
{
	for (int actorIndex = 0; actorIndex < static_cast<int>(m_actors.size()); actorIndex++)
	{
		if (!IsActorAlive(m_actors[actorIndex]) || m_actors[actorIndex]->m_isStatic)
		{
			continue;
		}

		CollideActorWithMap(m_actors[actorIndex]);
	}
}

void Map::CollideActorWithMap(Actor* actor)
{
	IntVec2 tileCoords = IntVec2(RoundDownToInt(actor->m_position.x), RoundDownToInt(actor->m_position.y));

	CollideActorWithTileIfSolid(actor, tileCoords + IntVec2::EAST);
	CollideActorWithTileIfSolid(actor, tileCoords + IntVec2::WEST);
	CollideActorWithTileIfSolid(actor, tileCoords + IntVec2::NORTH);
	CollideActorWithTileIfSolid(actor, tileCoords + IntVec2::SOUTH);

	CollideActorWithTileIfSolid(actor, tileCoords + IntVec2::EAST + IntVec2::NORTH);
	CollideActorWithTileIfSolid(actor, tileCoords + IntVec2::EAST + IntVec2::SOUTH);
	CollideActorWithTileIfSolid(actor, tileCoords + IntVec2::WEST + IntVec2::NORTH);
	CollideActorWithTileIfSolid(actor, tileCoords + IntVec2::WEST + IntVec2::SOUTH);

	CollideActorWithFloorAndCeiling(actor);
}

void Map::CollideActorWithTileIfSolid(Actor* actor, IntVec2 const& tileCoords)
{
	if (tileCoords.x < 0 || tileCoords.x > GetDimensions().x - 1 || tileCoords.y < 0 || tileCoords.y > GetDimensions().y - 1)
	{
		return;
	}

	int tileIndex = tileCoords.x + tileCoords.y * m_definition.m_dimensions.x;
	Tile& tile = m_tiles[tileIndex];

	if (!tile.IsSolid())
	{
		return;
	}

	Vec2 actorPositionXY = actor->m_position.GetXY();

	if (PushDiscOutOfFixedAABB2(actorPositionXY, actor->m_physicsRadius, tile.GetBounds()))
	{
		actor->OnCollide(nullptr);
	}
	actor->m_position = Vec3(actorPositionXY.x, actorPositionXY.y, actor->m_position.z);
}

void Map::CollideActorWithFloorAndCeiling(Actor* actor)
{
	if (actor->m_position.z < 0.f)
	{
		actor->m_position.z = 0.f;
		actor->OnCollide(nullptr);
	}
	else if (actor->m_position.z + actor->m_physicsHeight > 1.f)
	{
		actor->m_position.z = 1.f - actor->m_physicsHeight;
		actor->OnCollide(nullptr);
	}
}

bool Map::IsActorAlive(Actor* const& actor) const
{
	return actor && !actor->m_isDead;
}

bool Map::IsOwner(Actor* actorA, Actor* actorB) const
{
	if (actorB->m_ownerUID == ActorUID::INVALID)
	{
		return false;
	}

	return (actorB->m_ownerUID == actorA->m_UID);
}

void Map::DeleteDestroyedActors()
{
	for (int actorIndex = 0; actorIndex < (int)m_actors.size(); actorIndex++)
	{
		Actor*& actor = m_actors[actorIndex];
		if (actor && actor->m_isDestroyed)
		{
			delete actor;
			actor = nullptr;
		}
	}
}

void Map::SpawnPlayer(int)
{
	int spawnPointIndex = g_RNG->RollRandomIntInRange(0, (int)m_spawnPoints.size() - 1);
	SpawnInfo spawnInfo;
	spawnInfo.m_actor = "Soldier";
	spawnInfo.m_position = m_spawnPoints[spawnPointIndex]->m_position;
	spawnInfo.m_orientation = m_spawnPoints[spawnPointIndex]->m_orientation;
	Actor* marine = SpawnActor(spawnInfo);
	m_game->m_player->Possess(marine);
}

Actor* Map::SpawnActor(SpawnInfo spawnInfo)
{
	ActorUID actorUID = ActorUID(m_actorSalt, (unsigned int)m_actors.size());
	Actor* actor = new Actor(this, spawnInfo, actorUID);
	m_actors.push_back(actor);
	m_actorSalt++;

	if (actor->m_definition.m_aiEnabled)
	{
		Controller* aiController = new AI();
		aiController->m_map = this;
		aiController->Possess(actor);
		m_aiControllers.push_back(aiController);
	}

	return actor;
}

Actor* Map::CreateSpawnPoint(SpawnInfo spawnInfo)
{
	ActorUID spawnPointUID = ActorUID(m_actorSalt, (unsigned int)m_actors.size());
	Actor* spawnPoint = new Actor(this, spawnInfo, spawnPointUID);
	m_spawnPoints.push_back(spawnPoint);
	m_actorSalt++;

	return spawnPoint;
}

Actor* Map::GetActorByUID(ActorUID const& uid) const
{
	if (uid == ActorUID::INVALID)
	{
		return nullptr;
	}

	unsigned int index = uid.GetIndex();
	Actor* const& actor = m_actors[index];

	return (actor && actor->m_UID == uid ? actor : nullptr);
}

Actor* Map::GetClosestVisibleEnemy(Actor* seeker) const
{
	Actor* closestEnemy = nullptr;
	float closestEnemyDistance = FLT_MAX;

	for (int actorIndex = 0; actorIndex < (int)m_actors.size(); actorIndex++)
	{
		Actor* const& actor = m_actors[actorIndex];
		if (!actor || !IsActorAlive(actor))
		{
			continue;
		}
		if (actor->m_definition.m_faction == seeker->m_definition.m_faction || actor->m_definition.m_faction == Faction::INVALID)
		{
			continue;
		}
		if (IsPointInsideDirectedSector2D(actor->m_position.GetXY(), seeker->m_position.GetXY(), seeker->GetForwardNormal().GetXY(), seeker->m_definition.m_sightAngle, seeker->m_definition.m_sightRadius))
		{
			Vec3 directionToActor = (actor->m_position - seeker->m_position).GetNormalized();

			DoomRaycastResult result = RaycastVsWalls(seeker->GetEyePosition(), directionToActor, seeker->m_definition.m_sightRadius);
			if (result.m_impactDistance * result.m_impactDistance > GetDistanceSquared2D(actor->m_position.GetXY(), seeker->m_position.GetXY()))
			{
				float enemyDistance = GetDistance3D(actor->m_position, seeker->m_position);
				if (enemyDistance < closestEnemyDistance)
				{
					closestEnemyDistance = enemyDistance;
					closestEnemy = actor;
				}
			}
		}
	}

	return closestEnemy;
}

bool Map::HasLineOfSight(Actor const* seeker, Actor const* target) const
{
	Vec3 directionToTarget = (target->m_position - seeker->m_position).GetNormalized();

	DoomRaycastResult result = RaycastVsWalls(seeker->GetEyePosition(), directionToTarget, seeker->m_definition.m_sightRadius);
	return ((result.m_impactDistance * result.m_impactDistance) > GetDistanceSquared2D(target->m_position.GetXY(), seeker->m_position.GetXY()));
}

void Map::DebugPossessNext()
{
}

Player const* Map::GetCurrentRenderingPlayer() const
{
	return m_currentRenderingPlayer;
}

Particle* Map::SpawnParticle(Vec3 const& position, float radius, Rgba8 const& color, float lifetime)
{
	SpawnInfo particleSpawnInfo;
	particleSpawnInfo.m_actor = "Particle";
	particleSpawnInfo.m_position = position;
	particleSpawnInfo.m_orientation = EulerAngles::ZERO;
	Particle* newParticle = new Particle(this, particleSpawnInfo, radius, color, lifetime);
	m_visualActors.push_back(newParticle);
	return newParticle;
}
