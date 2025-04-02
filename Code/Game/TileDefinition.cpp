#include "Game/TileDefinition.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"

std::map<std::string, TileDefinition> TileDefinition::s_tileDefs;

void TileDefinition::InitializeTileDefinitions()
{
	XmlDocument tileDefsXmlFile("Data/Definitions/TileDefinitions.xml");
	XmlResult fileLoadResult = tileDefsXmlFile.LoadFile("Data/Definitions/TileDefinitions.xml");

	if (fileLoadResult != XmlResult::XML_SUCCESS)
	{
		ERROR_AND_DIE("Could not find or open file \"Data/Definitions/TileDefinitions.xml\"");
	}

	XmlElement* tileDefinitionsXmlElement = tileDefsXmlFile.RootElement();
	XmlElement* tileDefinitionXmlElement = tileDefinitionsXmlElement->FirstChildElement();

	while (tileDefinitionXmlElement)
	{
		TileDefinition tileDef(tileDefinitionXmlElement);
		s_tileDefs[tileDef.m_typeName] = tileDef;
		tileDefinitionXmlElement = tileDefinitionXmlElement->NextSiblingElement();
	}
}

TileDefinition::TileDefinition(XmlElement const* element)
{
	m_typeName = ParseXmlAttribute(*element, "name", "INVALID_TILE_TYPE");
	m_isSolid = ParseXmlAttribute(*element, "isSolid", false);
	m_floorSpriteCoords = ParseXmlAttribute(*element, "floorSpriteCoords", IntVec2::ZERO);
	m_ceilingSpriteCoords = ParseXmlAttribute(*element, "ceilingSpriteCoords", IntVec2::ZERO);
	m_wallSpriteCoords = ParseXmlAttribute(*element, "wallSpriteCoords", IntVec2::ZERO);
	m_tint = ParseXmlAttribute(*element, "tint", Rgba8::WHITE);
	m_mapImageColor = ParseXmlAttribute(*element, "mapImagePixelColor", m_mapImageColor);
}
