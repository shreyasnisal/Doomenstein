#pragma once

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVec2.hpp"

#include <string>

struct TileDefinition;

class Tile
{
public:
	~Tile() = default;
	Tile() = default;
	Tile(std::string typeName, IntVec2 tileCoords);
	Tile(std::string typeName, int x, int y);

	AABB2									GetBounds() const;
	std::string								GetType() const;
	IntVec2									GetFloorSpriteCoords() const;
	IntVec2									GetCeilingSpriteCoords() const;
	IntVec2									GetWallSpriteCoords() const;
	Rgba8									GetColor() const;
	bool									IsSolid() const;

public:
	TileDefinition*							m_tileDefinition;
	IntVec2									m_tileCoords;
};
