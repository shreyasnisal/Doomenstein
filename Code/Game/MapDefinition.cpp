#include "Game/MapDefinition.hpp"

#include "Game/GameCommon.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"


std::map<std::string, MapDefinition> MapDefinition::s_mapDefs;

void MapDefinition::InitializeMapDefinitions()
{
	XmlDocument mapDefinitionsXmlFile("Data/Definitions/MapDefinitions.xml");
	XmlResult xmlFileLoadResult = mapDefinitionsXmlFile.LoadFile("Data/Definitions/MapDefinitions.xml");
	if (xmlFileLoadResult != XmlResult::XML_SUCCESS)
	{
		ERROR_AND_DIE("Could not find or open file Data/Definitions/MapDefinitions.xml");
	}

	XmlElement* mapDefinitionsXmlElement = mapDefinitionsXmlFile.RootElement();
	XmlElement* mapDefinitionXmlElement = mapDefinitionsXmlElement->FirstChildElement();

	while (mapDefinitionXmlElement)
	{
		MapDefinition mapDef = MapDefinition(mapDefinitionXmlElement);
		s_mapDefs[mapDef.m_name] = mapDef;
		mapDefinitionXmlElement = mapDefinitionXmlElement->NextSiblingElement();
	}
}

MapDefinition::MapDefinition(XmlElement const* element)
{
	m_name = ParseXmlAttribute(*element, "name", m_name);
	m_imagePath = ParseXmlAttribute(*element, "image", m_imagePath);
	std::string spriteSheetTexturePath = ParseXmlAttribute(*element, "spriteSheetTexture", "");
	m_spriteSheetDimensions = ParseXmlAttribute(*element, "spriteSheetCellCount", IntVec2::ZERO);
	Texture* spriteSheetTexture = g_renderer->CreateOrGetTextureFromFile(spriteSheetTexturePath.c_str());
	m_terrainSpriteSheet = new SpriteSheet(spriteSheetTexture, m_spriteSheetDimensions);
	
	std::string shaderPath = ParseXmlAttribute(*element, "shader", "");
	if (!shaderPath.empty())
	{
		m_shader = g_renderer->CreateOrGetShader(shaderPath.c_str(), VertexType::VERTEX_PCUTBN);
	}

	XmlElement const* spawnInfosXmlElement = element->FirstChildElement("SpawnInfos");
	XmlElement const* spawnInfoXmlElement = spawnInfosXmlElement->FirstChildElement();
	while (spawnInfoXmlElement)
	{
		m_spawnInfos.push_back(SpawnInfo(spawnInfoXmlElement));
		spawnInfoXmlElement = spawnInfoXmlElement->NextSiblingElement();
	}
}

SpawnInfo::SpawnInfo(XmlElement const* element)
{
	//std::string actorDefName = ParseXmlAttribute(*element, "actor", "");
	//m_actor = ActorDefinition::s_actorDefs[actorDefName];
	m_actor = ParseXmlAttribute(*element, "actor", "");
	m_position = ParseXmlAttribute(*element, "position", m_position);
	m_orientation = ParseXmlAttribute(*element, "orientation", m_orientation);
}
