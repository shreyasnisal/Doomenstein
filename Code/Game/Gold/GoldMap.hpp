#pragma once

#include "Game/Map.hpp"
#include "Game/Gold/StaticActor.hpp"

#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"

#include <vector>


class Actor;
class Player;
class Game;

class GoldMap : public Map
{
public:
	const int SOLDIERS_IN_WAVE[3] = {10, 15, 20};
	const int TANKS_IN_WAVE[3] = {0, 0, 3};

public:
	~GoldMap();
	GoldMap(Game* game);

	void PlaceCliffs();
	void PlaceTrees();
	void PlaceRocks();

	void SpawnWave();
	void ShowLevelMessage();
	void IncrementLevel();
	void HandleWaveStart();

	virtual void Update() override;
	virtual void Render() const override;
	virtual void RenderScreen() const override;
	virtual void RenderCustomScreens() const override;
	void RenderScene() const;
	virtual void RenderStaticActors() const;
	void RenderVisualActors() const;
	void UpdateVisualActors();

	void CollideActorsWithStaticActors();
	virtual void CollideActorWithFloorAndCeiling(Actor* actor) override;

	//virtual void SpawnPlayer(int playerIndex) override;
	//virtual Actor* SpawnActor(SpawnInfo spawnInfo) override;

	bool IsValidSpawnLocation(float x, float y) const;
	void UpdateActorPivotPositions();

	virtual void DeleteDestroyedActors() override;

	//virtual DoomRaycastResult		RaycastVsActors(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDistance, Actor* actorToExclude = nullptr) const;
	virtual DoomRaycastResult		RaycastVsWalls(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDistance) const;	virtual Player* GetCurrentRenderingPlayer() const;

public:
	IntVec2 m_dimensions = IntVec2::ZERO;
	Shader* m_shader = nullptr;
	Model* m_blockModel = nullptr;
	std::vector<StaticActor*> m_staticActors;
	Texture* m_skyboxTexture = nullptr;

	Texture* m_renderTargetTexture = nullptr;
	VertexBuffer* m_fullscreenVBO = nullptr;
	Texture* m_shadowMap = nullptr;
	Shader* m_shadowShader = nullptr;
	Shader* m_diffuseShader = nullptr;

	int m_remainingEnemies = 0;
	int m_level = 0;
	bool m_isCombatMode = false;
};