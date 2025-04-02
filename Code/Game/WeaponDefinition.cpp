#include "Game/WeaponDefinition.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"

#include "Game/ActorDefinition.hpp"
#include "Game/GameCommon.hpp"

std::map<std::string, WeaponDefinition> WeaponDefinition::s_weaponDefs;

void WeaponDefinition::InitializeWeaponDefinitions()
{
	XmlDocument weaponDefsXmlFile("Data/Definitions/WeaponDefinitions.xml");
	XmlResult fileLoadResult = weaponDefsXmlFile.LoadFile("Data/Definitions/WeaponDefinitions.xml");

	if (fileLoadResult != XmlResult::XML_SUCCESS)
	{
		ERROR_AND_DIE("Could not find or open file \"Data/Definitions/WeaponDefinitions.xml\"");
	}

	XmlElement* weaponDefinitionsXmlElement = weaponDefsXmlFile.RootElement();
	XmlElement* weaponDefXmlElement = weaponDefinitionsXmlElement->FirstChildElement();

	while (weaponDefXmlElement)
	{
		WeaponDefinition weaponDef(weaponDefXmlElement);
		s_weaponDefs[weaponDef.m_name] = weaponDef;
		weaponDefXmlElement = weaponDefXmlElement->NextSiblingElement();
	}
}

WeaponDefinition::WeaponDefinition(XmlElement const* element)
{
	m_name = ParseXmlAttribute(*element, "name", m_name);
	m_refireTime = ParseXmlAttribute(*element, "refireTime", m_refireTime);
	
	m_rayCount = ParseXmlAttribute(*element, "rayCount", m_rayCount);
	m_rayCone = ParseXmlAttribute(*element, "rayCone", m_rayCone);
	m_rayRange = ParseXmlAttribute(*element, "rayRange", m_rayRange);
	m_rayDamage = ParseXmlAttribute(*element, "rayDamage", m_rayDamage);
	m_rayImpulse = ParseXmlAttribute(*element, "rayImpulse", m_rayImpulse);

	m_projectileCount = ParseXmlAttribute(*element, "projectileCount", m_projectileCount);
	m_projectileActor = ParseXmlAttribute(*element, "projectileActor", "");
	m_projectileCone = ParseXmlAttribute(*element, "projectileCone", m_projectileCone);
	m_projectileSpeed = ParseXmlAttribute(*element, "projectileSpeed", m_projectileSpeed);

	m_meleeCount = ParseXmlAttribute(*element, "meleeCount", m_meleeCount);
	m_meleeRange = ParseXmlAttribute(*element, "meleeRange", m_meleeRange);
	m_meleeDamage = ParseXmlAttribute(*element, "meleeDamage", m_meleeDamage);
	m_meleeImpulse = ParseXmlAttribute(*element, "meleeImpulse", m_meleeImpulse);
	m_meleeArc = ParseXmlAttribute(*element, "meleeArc", m_meleeArc);

	XmlElement const* visualsElement = element->FirstChildElement("Visuals");
	if (visualsElement)
	{
		m_is3DWeapon = ParseXmlAttribute(*visualsElement, "is3D", m_is3DWeapon);
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
		std::string modelFilepath = ParseXmlAttribute(*visualsElement, "modelFile", "");
		if (!modelFilepath.empty())
		{
			m_model = g_modelLoader->CreateOrGetModelFromObj(modelFilepath.c_str(), modelTransformMatrix);
		}
		std::string reticleTextureName = ParseXmlAttribute(*visualsElement, "reticleTexture", "");
		if (!reticleTextureName.empty())
		{
			m_reticleTexture = g_renderer->CreateOrGetTextureFromFile(reticleTextureName.c_str());
		}
		m_reticleSize = ParseXmlAttribute(*visualsElement, "reticleSize", m_reticleSize);
	}

	XmlElement const* hudElement = element->FirstChildElement("HUD");
	if (hudElement)
	{
		std::string textureName = ParseXmlAttribute(*hudElement, "baseTexture", "");
		if (!textureName.empty())
		{
			m_hudTexture = g_renderer->CreateOrGetTextureFromFile(textureName.c_str());
		}
		std::string reticleTextureName = ParseXmlAttribute(*hudElement, "reticleTexture", "");
		if (!reticleTextureName.empty())
		{
			m_reticleTexture = g_renderer->CreateOrGetTextureFromFile(reticleTextureName.c_str());
		}
		m_reticleSize = ParseXmlAttribute(*hudElement, "reticleSize", m_reticleSize);
		m_spriteSize = ParseXmlAttribute(*hudElement, "spriteSize", m_spriteSize);
		m_spritePivot = ParseXmlAttribute(*hudElement, "spritePivot", m_spritePivot);

		XmlElement const* animationElement = hudElement->FirstChildElement("Animation");
		while (animationElement)
		{
			std::string animationName = ParseXmlAttribute(*animationElement, "name", "");
			if (!strcmp(animationName.c_str(), "Idle"))
			{
				std::string idleAnimationShaderName = ParseXmlAttribute(*animationElement, "shader", "");
				if (!idleAnimationShaderName.empty())
				{
					m_idleAnimationShader = g_renderer->CreateOrGetShader(idleAnimationShaderName.c_str());
				}
				std::string idleAnimationTextureName = ParseXmlAttribute(*animationElement, "spriteSheet", "");
				Texture* idleAnimationTexture = nullptr;
				if (!idleAnimationTextureName.empty())
				{
					idleAnimationTexture = g_renderer->CreateOrGetTextureFromFile(idleAnimationTextureName.c_str());
				}
				IntVec2 idleAnimationSpriteSheetCellCount = ParseXmlAttribute(*animationElement, "cellCount", IntVec2::ZERO);
				float idleAnimationSecondsPerFrame = ParseXmlAttribute(*animationElement, "secondsPerFrame", 0.f);
				SpriteSheet* idleAnimationSpriteSheet = new SpriteSheet(idleAnimationTexture, idleAnimationSpriteSheetCellCount);
				m_idleAnimation = SpriteAnimDefinition(idleAnimationSpriteSheet, -1, -1, idleAnimationSecondsPerFrame, SpriteAnimPlaybackType::ONCE);
				m_idleAnimation.LoadFromXml(animationElement);
			}
			else if (!strcmp(animationName.c_str(), "Attack"))
			{
				std::string attackAnimationShaderName = ParseXmlAttribute(*animationElement, "shader", "");
				if (!attackAnimationShaderName.empty())
				{
					m_attackAnimationShader = g_renderer->CreateOrGetShader(attackAnimationShaderName.c_str());
				}
				std::string attackAnimationTextureName = ParseXmlAttribute(*animationElement, "spriteSheet", "");
				Texture* attackAnimationTexture = nullptr;
				if (!attackAnimationTextureName.empty())
				{
					attackAnimationTexture = g_renderer->CreateOrGetTextureFromFile(attackAnimationTextureName.c_str());
				}
				IntVec2 attackAnimationSpriteSheetCellCount = ParseXmlAttribute(*animationElement, "cellCount", IntVec2::ZERO);
				float attackAnimationSecondsPerFrame = ParseXmlAttribute(*animationElement, "secondsPerFrame", 0.f);
				SpriteSheet* attackAnimationSpriteSheet = new SpriteSheet(attackAnimationTexture, attackAnimationSpriteSheetCellCount);
				m_attackAnimation = SpriteAnimDefinition(attackAnimationSpriteSheet, -1, -1, attackAnimationSecondsPerFrame, SpriteAnimPlaybackType::ONCE);
				m_attackAnimation.LoadFromXml(animationElement);
			}

			animationElement = animationElement->NextSiblingElement();
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
			if (!strcmp(soundName.c_str(), "Fire"))
			{
				std::string fireSoundPath = ParseXmlAttribute(*soundElement, "name", "");
				if (!fireSoundPath.empty())
				{
					m_fireSound = g_audio->CreateOrGetSound(fireSoundPath, true);
				}
			}
			soundElement = soundElement->NextSiblingElement();
		}
	}
}
