#pragma once

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/XMLUtils.hpp"

#include<map>
#include <string>

struct TileDefinition
{
public:
	std::string					m_typeName;
	AABB2						m_uvs = AABB2(Vec2::ZERO, Vec2::ONE);
	Rgba8						m_tint = Rgba8::WHITE;
	Rgba8						m_mapImageColor = Rgba8::TRANSPARENT_BLACK;
	IntVec2						m_floorSpriteCoords = IntVec2::ZERO;
	IntVec2						m_ceilingSpriteCoords = IntVec2::ZERO;
	IntVec2						m_wallSpriteCoords = IntVec2::ZERO;
	bool						m_isSolid = false;

public:
	~TileDefinition() = default;
	TileDefinition() = default;
	TileDefinition(XmlElement const* element);

	static void					InitializeTileDefinitions();

	static std::map<std::string, TileDefinition>		s_tileDefs;
};
