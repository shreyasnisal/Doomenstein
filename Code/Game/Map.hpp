#pragma once

#include "Game/ActorUID.hpp"
#include "Game/App.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/Tile.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Core/HeatMaps/TileHeatMap.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"

#include <vector>

class Actor;
class Game;
class Particle;
class Player;
class VertexBuffer;
class IndexBuffer;
class Controller;

class Map
{
public:
	virtual ~Map();
	Map() = default;
	Map(Game* game, MapDefinition mapDef);

	virtual void			Update();
	virtual void			UpdateActors();

	virtual void			Render() const;
	virtual void			RenderCustomScreens() const = 0;
	virtual void			RenderScreen() const = 0;
	virtual void			RenderTiles() const;
	virtual void			RenderActors() const;

	void					ConstructMapFromImage();
	void					CreateTileHeatMaps();
	void					InitializeTiles();
	virtual IntVec2			GetDimensions() const { return m_definition.m_dimensions; }

	void					SetTileType(IntVec2 const& tileCoords, std::string tileTypeName);
	void					AddVertsForTile(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, int tileIndex, AABB2 const& floorUVs, AABB2 const& ceilingUVs) const;
	void					AddVertsForWall(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, int tileIndex, AABB2 const& wallUVs) const;

	bool					IsActorAlive(Actor* const& actor) const;
	bool					IsOwner(Actor* actorA, Actor* actorB) const;

	virtual DoomRaycastResult		RaycastVsAll(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDistance, Actor* actorToExclude = nullptr) const;
	virtual DoomRaycastResult		RaycastVsActors(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDistance, Actor* actorToExclude = nullptr) const;
	virtual DoomRaycastResult		RaycastVsWalls(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDistance) const;

	virtual void					CollideActors();
	virtual void					CollideActors(Actor* actorA, Actor* actorB);
	virtual void					CollideActorsWithMap();
	virtual void					CollideActorWithMap(Actor* actor);
	virtual void					CollideActorWithTileIfSolid(Actor* actor, IntVec2 const& tileCoords);
	virtual void					CollideActorWithFloorAndCeiling(Actor* actor);

	virtual void					DeleteDestroyedActors();
	virtual void					SpawnPlayer(int playerIndex);
	virtual Actor*					SpawnActor(SpawnInfo spawnInfo);
	virtual Actor*					CreateSpawnPoint(SpawnInfo spawnInfo);
	virtual Actor*					GetActorByUID(ActorUID const& uid) const;
	virtual Actor*					GetClosestVisibleEnemy(Actor* seeker) const;
	bool							HasLineOfSight(Actor const* seeker, Actor const* target) const;
	virtual void					DebugPossessNext();

	virtual Player const*			GetCurrentRenderingPlayer() const;

	Particle* SpawnParticle(Vec3 const& positiion, float radius, Rgba8 const& color, float lifetime);

public:
	Game* m_game;
	MapDefinition m_definition;
	std::vector<Tile> m_tiles;
	std::vector<Actor*> m_actors;
	std::vector<Actor*> m_spawnPoints;
	std::vector<Actor*> m_visualActors;
	TileHeatMap* m_solidMap = nullptr;
	VertexBuffer* m_tileVertexBuffer = nullptr;
	IndexBuffer* m_tileIndexBuffer = nullptr;
	unsigned int m_actorSalt = 0;
	std::vector<Controller*> m_aiControllers;
	Player* m_currentRenderingPlayer = nullptr;
};
