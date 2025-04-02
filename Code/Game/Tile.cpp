#include "Game/Tile.hpp"

#include "Game/TileDefinition.hpp"

Tile::Tile(std::string typeName, IntVec2 tileCoords)
{
	m_tileDefinition = &(TileDefinition::s_tileDefs.find(typeName)->second);
	m_tileCoords = tileCoords;
}

Tile::Tile(std::string typeName, int x, int y)
{
	m_tileDefinition = &(TileDefinition::s_tileDefs.find(typeName)->second);
	m_tileCoords = IntVec2(x, y);
}

AABB2 Tile::GetBounds() const
{
	AABB2 bounds;
	bounds.m_mins.x = static_cast<float>(m_tileCoords.x);
	bounds.m_mins.y = static_cast<float>(m_tileCoords.y);
	bounds.m_maxs.x = static_cast<float>(m_tileCoords.x + 1);
	bounds.m_maxs.y = static_cast<float>(m_tileCoords.y + 1);
	return bounds;
}

Rgba8 Tile::GetColor() const
{
	return m_tileDefinition->m_tint;
}

bool Tile::IsSolid() const
{
	return m_tileDefinition->m_isSolid;
}

std::string Tile::GetType() const
{
	return m_tileDefinition->m_typeName;
}

IntVec2 Tile::GetFloorSpriteCoords() const
{
	return m_tileDefinition->m_floorSpriteCoords;
}

IntVec2 Tile::GetCeilingSpriteCoords() const
{
	return m_tileDefinition->m_ceilingSpriteCoords;
}

IntVec2 Tile::GetWallSpriteCoords() const
{
	return m_tileDefinition->m_wallSpriteCoords;
}
