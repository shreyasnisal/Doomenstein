#include "Game/ActorDefinition.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Weapon.hpp"
#include "Game/WeaponDefinition.hpp"


std::map<std::string, ActorDefinition> ActorDefinition::s_actorDefs;

Faction GetFactionFromString(std::string factionString)
{
	if (!strcmp("Marine", factionString.c_str()))
	{
		return Faction::MARINE;
	}
	else if (!strcmp("Demon", factionString.c_str()))
	{
		return Faction::DEMON;
	}
	else if (!strcmp("SpawnPoint", factionString.c_str()))
	{
		return Faction::SPAWNPOINT;
	}

	return Faction::INVALID;
}

BillboardType GetBillboardTypeFromString(std::string billboardTypeStr)
{
	if (!strcmp("WorldUpFacing", billboardTypeStr.c_str()))
	{
		return BillboardType::WORLD_UP_FACING;
	}
	if (!strcmp("WorldUpOpposing", billboardTypeStr.c_str()))
	{
		return BillboardType::WORLD_UP_OPPOSING;
	}
	if (!strcmp("FullFacing", billboardTypeStr.c_str()))
	{
		return BillboardType::FULL_FACING;
	}
	if (!strcmp("FullOpposing", billboardTypeStr.c_str()))
	{
		return BillboardType::FULL_OPPOSING;
	}
	return BillboardType::NONE;
}

void ActorDefinition::InitializeActorDefinitions()
{
	XmlDocument actorDefsXmlFile("Data/Definitions/ActorDefinitions.xml");
	XmlResult fileLoadResult = actorDefsXmlFile.LoadFile("Data/Definitions/ActorDefinitions.xml");

	if (fileLoadResult != XmlResult::XML_SUCCESS)
	{
		ERROR_AND_DIE("Could not find or open file \"Data/Definitions/ActorDefinitions.xml\"");
	}

	XmlElement* actorDefinitionsXmlElement = actorDefsXmlFile.RootElement();
	XmlElement* actorDefXmlElement = actorDefinitionsXmlElement->FirstChildElement();

	while (actorDefXmlElement)
	{
		ActorDefinition actorDef(actorDefXmlElement);
		s_actorDefs[actorDef.m_name] = actorDef;
		actorDefXmlElement = actorDefXmlElement->NextSiblingElement();
	}

	fileLoadResult = actorDefsXmlFile.LoadFile("Data/Definitions/ProjectileActorDefinitions.xml");

	if (fileLoadResult != XmlResult::XML_SUCCESS)
	{
		ERROR_AND_DIE("Could not find or open file \"Data/Definitions/ProjectileActorDefinitions.xml\"");
	}

	actorDefinitionsXmlElement = actorDefsXmlFile.RootElement();
	actorDefXmlElement = actorDefinitionsXmlElement->FirstChildElement();

	while (actorDefXmlElement)
	{
		ActorDefinition actorDef(actorDefXmlElement);
		s_actorDefs[actorDef.m_name] = actorDef;
		actorDefXmlElement = actorDefXmlElement->NextSiblingElement();
	}
}

ActorDefinition::ActorDefinition(XmlElement const* element)
{
	m_name = ParseXmlAttribute(*element, "name", m_name);
	std::string factionString = ParseXmlAttribute(*element, "faction", "INVALID");
	m_faction = GetFactionFromString(factionString);
	m_health = ParseXmlAttribute(*element, "health", m_health);
	m_canBePossessed = ParseXmlAttribute(*element, "canBePossessed", m_canBePossessed);
	m_dieOnSpawn = ParseXmlAttribute(*element, "dieOnSpawn", m_dieOnSpawn);
	m_corpseLifetime = ParseXmlAttribute(*element, "corpseLifetime", m_corpseLifetime);
	m_visible = ParseXmlAttribute(*element, "visible", m_visible);
	m_solidColor = ParseXmlAttribute(*element, "solidColor", m_solidColor);
	m_wireframeColor = ParseXmlAttribute(*element, "wireframeColor", m_wireframeColor);

	XmlElement const* collisionElement = element->FirstChildElement("Collision");
	if (collisionElement)
	{
		m_physicsRadius = ParseXmlAttribute(*collisionElement, "radius", m_physicsRadius);
		m_physicsHeight = ParseXmlAttribute(*collisionElement, "height", m_physicsHeight);
		m_collidesWithWorld = ParseXmlAttribute(*collisionElement, "collidesWithWorld", m_collidesWithWorld);
		m_collidesWithActors = ParseXmlAttribute(*collisionElement, "collidesWithActors", m_collidesWithActors);
		m_dieOnCollide = ParseXmlAttribute(*collisionElement, "dieOnCollide", m_dieOnCollide);
		m_damageOnCollide = ParseXmlAttribute(*collisionElement, "damageOnCollide", m_damageOnCollide);
		m_impulseOnCollide = ParseXmlAttribute(*collisionElement, "impulseOnCollide", m_impulseOnCollide);

		m_explodeOnDie = ParseXmlAttribute(*collisionElement, "explodeOnDie", m_explodeOnDie);
		m_explosionRadius = ParseXmlAttribute(*collisionElement, "explosionRadius", m_explosionRadius);
		m_explosionParticles = ParseXmlAttribute(*collisionElement, "explosionParticles", m_explosionParticles);
		m_explosionParticleColor = ParseXmlAttribute(*collisionElement, "explosionParticleColor", m_explosionParticleColor);
		m_explosionParticleSize = ParseXmlAttribute(*collisionElement, "explosionParticleSize", m_explosionParticleSize);
		m_explosionParticleLifetime = ParseXmlAttribute(*collisionElement, "explosionParticleLifetime", m_explosionParticleLifetime);
		m_explosionParticleSpeed = ParseXmlAttribute(*collisionElement, "explosionParticleSpeed", m_explosionParticleSpeed);
		m_explosionDamage = ParseXmlAttribute(*collisionElement, "damageOnExplode", m_explosionDamage);
		m_impulseOnExplode = ParseXmlAttribute(*collisionElement, "impulseOnExplode", m_impulseOnExplode);
	}

	XmlElement const* physicsElement = element->FirstChildElement("Physics");
	if (physicsElement)
	{
		m_simulated = ParseXmlAttribute(*physicsElement, "simulated", m_simulated);
		m_walkSpeed = ParseXmlAttribute(*physicsElement, "walkSpeed", m_walkSpeed);
		m_runSpeed = ParseXmlAttribute(*physicsElement, "runSpeed", m_runSpeed);
		m_turnSpeed = ParseXmlAttribute(*physicsElement, "turnSpeed", m_turnSpeed);
		m_drag = ParseXmlAttribute(*physicsElement, "drag", m_drag);
		m_gravityScale = ParseXmlAttribute(*physicsElement, "gravityScale", m_gravityScale);
	}

	XmlElement const* cameraElement = element->FirstChildElement("Camera");
	if (cameraElement)
	{
		m_eyeHeight = ParseXmlAttribute(*cameraElement, "eyeHeight", m_eyeHeight);
		m_eyeFov = ParseXmlAttribute(*cameraElement, "cameraFOV", m_eyeFov);
	}

	XmlElement const* aiElement = element->FirstChildElement("AI");
	if (aiElement)
	{
		m_aiEnabled = ParseXmlAttribute(*aiElement, "aiEnabled", m_aiEnabled);
		m_sightRadius = ParseXmlAttribute(*aiElement, "sightRadius", m_sightRadius);
		m_sightAngle = ParseXmlAttribute(*aiElement, "sightAngle", m_sightAngle);
	}

	// Add weapons from inventory
	XmlElement const* inventoryElement = element->FirstChildElement("Inventory");
	if (inventoryElement)
	{
		m_weaponHeight = ParseXmlAttribute(*inventoryElement, "weaponHeight", m_weaponHeight);
		XmlElement const* weaponElement = inventoryElement->FirstChildElement("Weapon");
		while (weaponElement)
		{
			std::string weaponName = ParseXmlAttribute(*weaponElement, "name", "invalid weapon");
			m_weapons.push_back(new Weapon(WeaponDefinition::s_weaponDefs[weaponName]));
			weaponElement = weaponElement->NextSiblingElement();
		}
	}

	// Add Visuals data
	XmlElement const* visualsElement = element->FirstChildElement("Visuals");
	if (visualsElement)
	{
		m_is3DActor = ParseXmlAttribute(*visualsElement, "is3D", m_is3DActor);
		if (m_is3DActor)
		{
			m_modelScale = ParseXmlAttribute(*visualsElement, "scale", 1.f);
			Mat44 modelTransformMatrix = Mat44::IDENTITY;
			XmlElement const* transformElement = visualsElement->FirstChildElement("Transform");
			if (transformElement)
			{
				modelTransformMatrix = Mat44(transformElement);
			}
			std::string shaderName = ParseXmlAttribute(*visualsElement, "shader", "Default");
			m_shader = g_renderer->CreateOrGetShader(shaderName.c_str(), VertexType::VERTEX_PCUTBN);
			std::string modelTextureFilepath = ParseXmlAttribute(*visualsElement, "texture", "");

			if (!modelTextureFilepath.empty())
			{
				m_texture = g_renderer->CreateOrGetTextureFromFile(modelTextureFilepath.c_str());
			}
			std::string modelFilePath = ParseXmlAttribute(*visualsElement, "modelFile", "");
			if (!modelFilePath.empty())
			{
				m_model = g_modelLoader->CreateOrGetModelFromObj(modelFilePath.c_str(), modelTransformMatrix);
			}

			m_showVisualParticles = ParseXmlAttribute(*visualsElement, "showParticles", m_showVisualParticles);
			m_visualParticles = ParseXmlAttribute(*visualsElement, "particles", m_visualParticles);
			m_visualParticleColor = ParseXmlAttribute(*visualsElement, "particleColor", m_visualParticleColor);
			m_visualParticleSize = ParseXmlAttribute(*visualsElement, "particleSize", m_visualParticleSize);
			m_visualParticleLifetime = ParseXmlAttribute(*visualsElement, "particleLifetime", m_visualParticleLifetime);
			m_visualParticleSpeed = ParseXmlAttribute(*visualsElement, "particleSpeed", m_visualParticleSpeed);
		}
		else
		{
			m_size = ParseXmlAttribute(*visualsElement, "size", m_size);
			m_pivot = ParseXmlAttribute(*visualsElement, "pivot", m_pivot);
			std::string billboardTypeStr = ParseXmlAttribute(*visualsElement, "billboardType", "None");
			m_billboardType = GetBillboardTypeFromString(billboardTypeStr);
			m_isLit = ParseXmlAttribute(*visualsElement, "renderLit", m_isLit);
			m_isRounded = ParseXmlAttribute(*visualsElement, "renderRounded", m_isRounded);
			std::string shaderName = ParseXmlAttribute(*visualsElement, "shader", "Default");
			m_shader = g_renderer->CreateOrGetShader(shaderName.c_str(), m_isLit ? VertexType::VERTEX_PCUTBN : VertexType::VERTEX_PCU);
			std::string textureName = ParseXmlAttribute(*visualsElement, "spriteSheet", "");
			m_texture = g_renderer->CreateOrGetTextureFromFile(textureName.c_str());
			m_spriteSheetCellCount = ParseXmlAttribute(*visualsElement, "cellCount", m_spriteSheetCellCount);
			if (m_texture)
			{
				m_spriteSheet = new SpriteSheet(m_texture, m_spriteSheetCellCount);
			}

			XmlElement const* animationGroupElement = visualsElement->FirstChildElement("AnimationGroup");
			while (animationGroupElement)
			{
				AnimationGroupDefinition animationGroup = AnimationGroupDefinition(animationGroupElement, m_spriteSheet);
				m_animations.push_back(animationGroup);
				animationGroupElement = animationGroupElement->NextSiblingElement();
			}
		}
	}

	// Add sound data
	XmlElement const* soundsElement = element->FirstChildElement("Sounds");
	if (soundsElement)
	{
		XmlElement const* soundElement = soundsElement->FirstChildElement();
		while (soundElement)
		{
			std::string soundName = ParseXmlAttribute(*soundElement, "sound", "");
			if (!strcmp(soundName.c_str(), "Hurt"))
			{
				std::string hurtSoundPath = ParseXmlAttribute(*soundElement, "name", "");
				if (!hurtSoundPath.empty())
				{
					m_hurtSound = g_audio->CreateOrGetSound(hurtSoundPath, true);
				}
			}
			else if (!strcmp(soundName.c_str(), "Death"))
			{
				std::string deathSoundPath = ParseXmlAttribute(*soundElement, "name", "");
				if (!deathSoundPath.empty())
				{
					m_deathSound = g_audio->CreateOrGetSound(deathSoundPath, true);
				}
			}
			else if (!strcmp(soundName.c_str(), "See"))
			{
				std::string seeSoundPath = ParseXmlAttribute(*soundElement, "name", "");
				if (!seeSoundPath.empty())
				{
					m_seeSound = g_audio->CreateOrGetSound(seeSoundPath, true);
				}
			}
			soundElement = soundElement->NextSiblingElement();
		}
	}
}

AnimationGroupDefinition ActorDefinition::GetAnimationGroupByName(std::string animationGroupName) const
{
	for (int animationGroupIndex = 0; animationGroupIndex < (int)m_animations.size(); animationGroupIndex++)
	{
		if (!strcmp(animationGroupName.c_str(), m_animations[animationGroupIndex].m_name.c_str()))
		{
			return m_animations[animationGroupIndex];
		}
	}

	return m_animations[0];
}
