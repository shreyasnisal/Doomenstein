#include "Game/Weapon.hpp"

#include "Game/Actor.hpp"
#include "Game/Controller.hpp"
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/Gold/Particle.hpp"
#include "Game/Player.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/VirtualReality/OpenXR.hpp"


Weapon::Weapon(WeaponDefinition const& definition, XRHand equipHand)
	: m_definition(definition)
	, m_equipHand(equipHand)
{
	m_currentAnimation = m_definition.m_idleAnimation;
	m_currentShader = m_definition.m_idleAnimationShader;
}

void Weapon::Update()
{
}

void Weapon::Render() const
{
	if (!m_definition.m_is3DWeapon)
	{
		return;
	}

	if (!m_definition.m_model)
	{
		return;
	}

	Actor* owner = m_map->GetActorByUID(m_ownerUID);
	if (!owner)
	{
		return;
	}

	Mat44 transform = Mat44::CreateTranslation3D(owner->GetWeaponPosition());
	transform.Append(owner->m_orientation.GetAsMatrix_iFwd_jLeft_kUp());
	transform.AppendScaleUniform3D(m_definition.m_modelScale);

	g_renderer->SetBlendMode(BlendMode::OPAQUE);
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_BACK);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetDepthMode(DepthMode::ENABLED);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->SetModelConstants(transform);
	g_renderer->BindTexture(m_definition.m_texture);
	//g_theRenderer->BindShader(m_definition.m_shader);
	g_renderer->DrawIndexBuffer(m_definition.m_model->GetVertexBuffer(), m_definition.m_model->GetIndexBuffer(), m_definition.m_model->GetIndexCount());
}

void Weapon::OnEquipped(Actor* owner)
{
	m_map = owner->m_map;
	m_ownerUID = owner->m_UID;
	m_refireTimer = Stopwatch(&m_map->m_game->m_gameClock, m_definition.m_refireTime);
	m_refireTimer.Start();
	m_animationClock = new Clock(owner->m_map->m_game->m_gameClock);
	m_animationClock->Reset();
}

void Weapon::Fire()
{
	if (!m_refireTimer.DecrementDurationIfElapsed())
	{
		return;
	}

	Actor* owner = m_map->GetActorByUID(m_ownerUID);
	if (!owner)
	{
		return;
	}

	g_audio->StartSoundAt(m_definition.m_fireSound, owner->m_position);

	if (!g_openXR || !g_openXR->IsInitialized())
	{
		if (owner->m_controller && owner->m_controller->IsPlayer())
		{
			owner->m_orientation.m_pitchDegrees -= m_definition.m_recoilAngle;
		}
	}

	Vec3 actorFwd = owner->GetRenderModelMatrix().GetIBasis3D();
	Vec3 eyePosition = owner->GetEyePosition() + actorFwd * owner->m_physicsRadius;
	Vec3 firePosition = eyePosition + Vec3::GROUNDWARD * owner->m_definition.m_physicsHeight * 0.1f + actorFwd * 0.1f;

	if (g_openXR && g_openXR->IsInitialized() && owner->m_controller && owner->m_controller->IsPlayer())
	{
		Player* player = (Player*)owner->m_controller;

		if (m_equipHand == XRHand::LEFT)
		{
			actorFwd = player->m_leftControllerOrientation.GetAsMatrix_iFwd_jLeft_kUp().GetIBasis3D();
			eyePosition = player->m_leftControllerWorldPosition;
			firePosition = player->m_leftControllerWorldPosition;
		}
		if (m_equipHand == XRHand::RIGHT)
		{
			actorFwd = player->m_rightControllerOrientation.GetAsMatrix_iFwd_jLeft_kUp().GetIBasis3D();
			eyePosition = player->m_rightControllerWorldPosition;
			firePosition = player->m_rightControllerWorldPosition;
		}
	}

	for (int rayIndex = 0; rayIndex < m_definition.m_rayCount; rayIndex++)
	{
		for (int particleIndex = 0; particleIndex < m_definition.m_particlesOnHit; particleIndex++)
		{
			float particleRadius = g_RNG->RollRandomFloatInRange(0.003f, 0.005f);
			if (g_openXR && g_openXR->IsInitialized() && owner->m_controller && owner->m_controller->IsPlayer())
			{
				Player* player = (Player*)owner->m_controller;

				Vec3 weaponFwd, weaponLeft, weaponUp;
				if (m_equipHand == XRHand::LEFT)
				{
					player->m_leftControllerOrientation.GetAsVectors_iFwd_jLeft_kUp(weaponFwd, weaponLeft, weaponUp);
				}
				if (m_equipHand == XRHand::RIGHT)
				{
					player->m_rightControllerOrientation.GetAsVectors_iFwd_jLeft_kUp(weaponFwd, weaponLeft, weaponUp);
				}

				Particle* particle = m_map->SpawnParticle(firePosition + weaponFwd * 0.25f - weaponLeft * 0.04f + weaponUp * 0.05f, particleRadius, m_definition.m_fireParticleColor, 0.1f);
				Vec3 randomDirection = g_RNG->RollRandomVec3InRadius(Vec3::ZERO, 1.f);
				particle->AddImpulse(randomDirection);
			}
			else
			{
				Particle* particle = m_map->SpawnParticle(owner->GetWeaponPosition() + owner->GetForwardNormal() * 0.08f + - owner->GetLeftNormal() * 0.04f + owner->GetUpNormal() * 0.05f, particleRadius, m_definition.m_fireParticleColor, 0.1f);
				Vec3 randomDirection = GetRandomFireDirection(owner, 15.f);

				particle->AddImpulse(randomDirection);
			}
		}

		Vec3 fireDirection = GetRandomFireDirection(owner, m_definition.m_rayCone);
		if (g_openXR && g_openXR->IsInitialized() && owner->m_controller && owner->m_controller->IsPlayer())
		{
			fireDirection = actorFwd;
		}
		DoomRaycastResult result = m_map->RaycastVsAll(eyePosition, fireDirection, m_definition.m_rayRange, owner);
		
		Vec3 rayEndPosition = eyePosition + actorFwd * m_definition.m_rayRange;
		if (result.m_didImpact)
		{
			SpawnInfo bulletHitSpawnInfo;
			bulletHitSpawnInfo.m_actor = "BulletHit";
			bulletHitSpawnInfo.m_position = result.m_impactPosition;
			
			if (result.m_impactActorUID != ActorUID::INVALID)
			{
				Actor* impactActor = m_map->GetActorByUID(result.m_impactActorUID);
				if (!impactActor)
				{
					for (int particleIndex = 0; particleIndex < m_definition.m_particlesOnHit; particleIndex++)
					{
						float particleRadius = g_RNG->RollRandomFloatInRange(0.003f, 0.005f);
						Particle* particle = m_map->SpawnParticle(result.m_impactPosition, particleRadius, m_definition.m_hitParticleColor, 0.1f);
						Vec3 randomDirection = Vec3(g_RNG->RollRandomFloatInRange(-1.f, 1.f), g_RNG->RollRandomFloatInRange(-1.f, 1.f), g_RNG->RollRandomFloatInRange(-1.f, 1.f));
						randomDirection = randomDirection.GetNormalized() * g_RNG->RollRandomFloatInRange(1.f, 2.f);
						particle->AddImpulse(randomDirection);
					}
					continue;
				}
				for (int particleIndex = 0; particleIndex < m_definition.m_particlesOnHit; particleIndex++)
				{
					float particleRadius = g_RNG->RollRandomFloatInRange(0.003f, 0.005f);
					Particle* particle = m_map->SpawnParticle(result.m_impactPosition, particleRadius, Rgba8::RED, 0.1f);
					Vec3 randomDirection = Vec3(g_RNG->RollRandomFloatInRange(-1.f, 1.f), g_RNG->RollRandomFloatInRange(-1.f, 1.f), g_RNG->RollRandomFloatInRange(-1.f, 1.f));
					randomDirection = randomDirection.GetNormalized() * g_RNG->RollRandomFloatInRange(1.f, 2.f);
					particle->AddImpulse(randomDirection);
				}
				float damage = g_RNG->RollRandomFloatInRange(m_definition.m_rayDamage);
				impactActor->TakeDamage(damage);
				if (impactActor->m_controller)
				{
					impactActor->m_controller->DamagedBy(owner);
				}
			}
			else
			{
				for (int particleIndex = 0; particleIndex < m_definition.m_particlesOnHit; particleIndex++)
				{
					float particleRadius = g_RNG->RollRandomFloatInRange(0.003f, 0.005f);
					Particle* particle = m_map->SpawnParticle(result.m_impactPosition, particleRadius, m_definition.m_hitParticleColor, 0.1f);
					Vec3 randomDirection = Vec3(g_RNG->RollRandomFloatInRange(-1.f, 1.f), g_RNG->RollRandomFloatInRange(-1.f, 1.f), g_RNG->RollRandomFloatInRange(-1.f, 1.f));
					randomDirection = randomDirection.GetNormalized() * g_RNG->RollRandomFloatInRange(1.f, 2.f);
					particle->AddImpulse(randomDirection);
				}
			}
		}
	}

	for (int projectileIndex = 0; projectileIndex < m_definition.m_projectileCount; projectileIndex++)
	{
		Vec3 fireDirection = GetRandomFireDirection(owner, m_definition.m_projectileCone);
		if (g_openXR && g_openXR->IsInitialized() && owner->m_controller && owner->m_controller->IsPlayer())
		{
			fireDirection = actorFwd;
		}
		SpawnInfo spawnInfo;
		spawnInfo.m_actor = m_definition.m_projectileActor;
		spawnInfo.m_position = firePosition;
		spawnInfo.m_orientation = owner->m_orientation;
		spawnInfo.m_orientation.m_pitchDegrees -= 10.f;
		Actor* projectileActor = m_map->SpawnActor(spawnInfo);
		projectileActor->m_ownerUID = m_ownerUID;
		projectileActor->AddImpulse(fireDirection * m_definition.m_projectileSpeed);
	}

	for (int meleeIndex = 0; meleeIndex < m_definition.m_meleeCount; meleeIndex++)
	{
		for (int actorIndex = 0; actorIndex < (int)m_map->m_actors.size(); actorIndex++)
		{
			Actor*& actor = m_map->m_actors[actorIndex];

			if (!actor)
			{
				continue;
			}

			if (actor->m_definition.m_faction == owner->m_definition.m_faction || actor->m_definition.m_faction == Faction::INVALID)
			{
				continue;
			}

			if (IsPointInsideDirectedSector2D(actor->m_position.GetXY(), owner->m_position.GetXY(), owner->GetForwardNormal().GetXY().GetNormalized(), m_definition.m_meleeArc, m_definition.m_meleeRange))
			{
				float damage = g_RNG->RollRandomFloatInRange(m_definition.m_meleeDamage);
				actor->TakeDamage(damage);
				Vec3 directionToTarget = (actor->m_position - owner->m_position).GetNormalized();
				actor->AddImpulse(directionToTarget * m_definition.m_meleeImpulse);
				if (actor->m_controller)
				{
					actor->m_controller->DamagedBy(owner);
				}
			}
		}
	}

	while (m_refireTimer.DecrementDurationIfElapsed());

	if (m_definition.m_is3DWeapon)
	{
		return;
	}

	m_currentShader = m_definition.m_attackAnimationShader;
	m_currentAnimation = m_definition.m_attackAnimation;
	m_animationClock->Reset();
	owner->m_currentAnimation = owner->m_definition.GetAnimationGroupByName("Attack");
	if (owner->m_currentAnimation.m_scaleBySpeed)
	{
		owner->m_animationClock.SetTimeScale(owner->m_velocity.GetLength() / owner->m_definition.m_runSpeed);
	}
	else
	{
		owner->m_animationClock.SetTimeScale(1.f);
	}
	owner->m_animationClock.Reset();
}

Vec3 const Weapon::GetRandomFireDirection(Actor const* actor, float deviationAngle) const
{
	Vec2 randomDeviation2 = g_RNG->RollRandomVec2InRadius(Vec2::ZERO, TanDegrees(deviationAngle));
	Vec3 randomDeviation3 = Vec3(0.f, randomDeviation2.x, randomDeviation2.y);
	Vec3 finalDirectionInActorLocalSpace = Vec3::EAST + randomDeviation3;
	return actor->GetRenderModelMatrix().TransformVectorQuantity3D(finalDirectionInActorLocalSpace);
}

float Weapon::GetRange() const
{
	if (m_definition.m_projectileCount != 0)
	{
		return FLT_MAX;
	}

	if (m_definition.m_rayCount != 0)
	{
		return m_definition.m_rayRange;
	}

	return m_definition.m_meleeRange;
}
