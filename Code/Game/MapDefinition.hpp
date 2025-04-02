#pragma once

#include "Game/ActorDefinition.hpp"

#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/Spritesheet.hpp"

#include <map>
#include <string>

struct SpawnInfo
{
public:
	std::string m_actor;
	Vec3 m_position = Vec3::ZERO;
	EulerAngles m_orientation = EulerAngles::ZERO;

	SpawnInfo() = default;
	SpawnInfo(XmlElement const* element);
};

struct MapDefinition
{
public:
	std::string				m_name = "";
	std::string				m_imagePath = "";
	IntVec2					m_dimensions = IntVec2::ZERO;
	SpriteSheet*			m_terrainSpriteSheet = nullptr;
	IntVec2					m_spriteSheetDimensions = IntVec2::ZERO;
	std::vector<SpawnInfo>	m_spawnInfos;
	Shader*					m_shader = nullptr;

public:
	~MapDefinition() = default;
	MapDefinition() = default;
	MapDefinition(XmlElement const* element);

	static void				InitializeMapDefinitions();

	static std::map<std::string, MapDefinition>		s_mapDefs;
};
