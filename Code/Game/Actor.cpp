#include "Game/Actor.hpp"

#include "Game/App.hpp"
#include "Game/Controller.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/Player.hpp"
#include "Game/Weapon.hpp"
#include "Game/Gold/Particle.hpp"

#include "Game/Gold/StaticActor.hpp"

#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/VirtualReality/OpenXR.hpp"

#include <vector>


Actor::~Actor()
{
	m_map->m_game->m_gameClock.RemoveChild(&m_animationClock);
}

Actor::Actor(Map* map, SpawnInfo const& spawnInfo, ActorUID uid)
	: m_map(map)
	, m_UID(uid)
	, m_position(spawnInfo.m_position)
	, m_orientation(spawnInfo.m_orientation)
	, m_animationClock(map->m_game->m_gameClock)
{
	m_definition = ActorDefinition::s_actorDefs[spawnInfo.m_actor];
	m_health = m_definition.m_health;
	m_physicsHeight = m_definition.m_physicsHeight;
	m_physicsRadius = m_definition.m_physicsRadius;
	m_pivotPosition = m_position + Vec3::SKYWARD * m_definition.m_weaponHeight;

	if (m_definition.m_dieOnSpawn)
	{
		Die();
	}
	
	for (int weaponIndex = 0; weaponIndex < (int)m_definition.m_weapons.size(); weaponIndex++)
	{
		m_weapons.push_back(new Weapon(m_definition.m_weapons[weaponIndex]->m_definition));
		m_leftWeapons.push_back(new Weapon(m_definition.m_weapons[weaponIndex]->m_definition, XRHand::LEFT));
		m_rightWeapons.push_back(new Weapon(m_definition.m_weapons[weaponIndex]->m_definition, XRHand::RIGHT));
	}

	if (!m_weapons.empty())
	{
		EquipWeapon(0);
	}
	
	if (!m_definition.m_is3DActor && m_definition.m_visible)
	{
		m_currentAnimation = m_definition.m_animations[0];
		if (m_currentAnimation.m_scaleBySpeed)
		{
			m_animationClock.SetTimeScale(m_velocity.GetLength() / m_definition.m_runSpeed);
		}
		else
		{
			m_animationClock.SetTimeScale(1.f);
		}
	}
}

void Actor::Update()
{
	if (m_isDestroyed)
	{
		return;
	}

	if (m_controller && !m_controller->IsPlayer())
	{
		m_controller->Update();
	}

	if (m_lifetimeTimer.HasDurationElapsed())
	{
		m_isDestroyed = true;
		if (m_controller && m_controller->IsPlayer() && m_definition.m_faction == Faction::MARINE)
		{
			Player* playerController = dynamic_cast<Player*>(m_controller);
			m_map->SpawnPlayer(playerController->m_playerIndex);
		}
	}

	if (m_equippedWeaponIndex != -1 && g_audio->IsPlaying(m_weapons[m_equippedWeaponIndex]->m_fireSoundPlayback))
	{
		g_audio->SetSoundPosition(m_weapons[m_equippedWeaponIndex]->m_fireSoundPlayback, m_position);
	}
	if (g_audio->IsPlaying(m_hurtSoundPlayback))
	{
		g_audio->SetSoundPosition(m_hurtSoundPlayback, m_position);
	}

	if (m_isDead)
	{
		return;
	}

	if (!m_currentAnimation.m_name.empty() && m_currentAnimation.m_animations[0].GetDuration() < m_animationClock.GetTotalSeconds() && m_currentAnimation.m_animations[0].GetPlaybackMode() == SpriteAnimPlaybackType::ONCE)
	{
		// Current Animation has ended
		// Reset to default "Walk" animation
		m_currentAnimation = m_definition.m_animations[0];

		if (m_currentAnimation.m_scaleBySpeed)
		{
			m_animationClock.SetTimeScale(m_velocity.GetLength() / m_definition.m_runSpeed);
		}
		else
		{
			m_animationClock.SetTimeScale(1.f);
		}
		m_animationClock.Reset();
	}

	UpdatePhysics();
	if (m_isGrounded)
	{
		m_orientation.m_pitchDegrees = GetClamped(m_orientation.m_pitchDegrees, -85.f, 45.f);
	}
	else
	{
		m_orientation.m_pitchDegrees = GetClamped(m_orientation.m_pitchDegrees, -85.f, 85.f);
	}

	if (m_definition.m_showVisualParticles)
	{
		for (int particleIndex = 0; particleIndex < m_definition.m_visualParticles; particleIndex++)
		{
			float particleSize = g_RNG->RollRandomFloatInRange(m_definition.m_visualParticleSize);
			Particle* particle = m_map->SpawnParticle(m_position - GetForwardNormal() * m_definition.m_physicsRadius, particleSize, m_definition.m_visualParticleColor, m_definition.m_visualParticleLifetime);
			Vec3 randomDirection = Vec3(g_RNG->RollRandomFloatInRange(-1.f, 1.f), g_RNG->RollRandomFloatInRange(-1.f, 1.f), g_RNG->RollRandomFloatInRange(-1.f, 1.f));
			randomDirection = randomDirection.GetNormalized() * m_definition.m_visualParticleSpeed;
			particle->AddImpulse(randomDirection);
		}
	}
}

void Actor::UpdatePhysics()
{
	float deltaSeconds = m_map->m_game->m_gameClock.GetDeltaSeconds();

	AddForce(-m_velocity * m_definition.m_drag);
	AddForce(Vec3::GROUNDWARD * GRAVITY * m_definition.m_gravityScale);

	m_velocity += m_acceleration * deltaSeconds;
	m_position += m_velocity * deltaSeconds;
	m_acceleration = Vec3::ZERO;

	if (m_position.z <= 0.f)
	{
		m_isGrounded = true;
	}

}

void Actor::Render() const
{

	if (g_openXR && g_openXR->IsInitialized())
	{
		if (m_controller && m_controller == m_map->GetCurrentRenderingPlayer())
		{
			Player const* player = m_map->GetCurrentRenderingPlayer();

			// Render Left Hand Weapon
			Weapon* const& leftWeapon = m_leftWeapons[player->m_leftWeaponIndex];
			Mat44 transform = Mat44::CreateTranslation3D(player->m_leftControllerWorldPosition);
			transform.Append(player->m_leftControllerOrientation.GetAsMatrix_iFwd_jLeft_kUp());
			transform.AppendScaleUniform3D(leftWeapon->m_definition.m_modelScale);

			g_renderer->SetBlendMode(BlendMode::OPAQUE);
			g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_BACK);
			g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
			g_renderer->SetDepthMode(DepthMode::ENABLED);
			g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
			g_renderer->SetModelConstants(transform);
			g_renderer->BindTexture(leftWeapon->m_definition.m_texture);
			//g_theRenderer->BindShader(m_definition.m_shader);
			g_renderer->DrawIndexBuffer(leftWeapon->m_definition.m_model->GetVertexBuffer(), leftWeapon->m_definition.m_model->GetIndexBuffer(), leftWeapon->m_definition.m_model->GetIndexCount());

			// Render Right Hand Weapon
			Weapon* const& rightWeapon = m_weapons[player->m_rightWeaponIndex];
			transform = Mat44::CreateTranslation3D(player->m_rightControllerWorldPosition);
			transform.Append(player->m_rightControllerOrientation.GetAsMatrix_iFwd_jLeft_kUp());
			transform.AppendScaleUniform3D(rightWeapon->m_definition.m_modelScale);

			g_renderer->SetBlendMode(BlendMode::OPAQUE);
			g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_BACK);
			g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
			g_renderer->SetDepthMode(DepthMode::ENABLED);
			g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
			g_renderer->SetModelConstants(transform);
			g_renderer->BindTexture(rightWeapon->m_definition.m_texture);
			//g_theRenderer->BindShader(m_definition.m_shader);
			g_renderer->DrawIndexBuffer(rightWeapon->m_definition.m_model->GetVertexBuffer(), rightWeapon->m_definition.m_model->GetIndexBuffer(), rightWeapon->m_definition.m_model->GetIndexCount());

			return;
		}
	}

	if (m_definition.m_is3DActor)
	{
		if (m_equippedWeaponIndex > -1)
		{
			m_weapons[m_equippedWeaponIndex]->Render();
		}

		Mat44 transform = Mat44::CreateTranslation3D(m_position);
		transform.Append(m_orientation.GetAsMatrix_iFwd_jLeft_kUp());

		g_renderer->SetBlendMode(m_definition.m_blendMode);
		g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_BACK);
		g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
		g_renderer->SetDepthMode(DepthMode::ENABLED);
		g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		g_renderer->SetModelConstants(transform);
		g_renderer->BindTexture(m_definition.m_texture);
		//g_theRenderer->BindShader(m_definition.m_shader);
		g_renderer->DrawIndexBuffer(m_definition.m_model->GetVertexBuffer(), m_definition.m_model->GetIndexBuffer(), m_definition.m_model->GetIndexCount());

		return;
	}

	if (m_controller && m_controller == m_map->GetCurrentRenderingPlayer())
	{
		return;
	}

	Mat44 billboardMatrix = GetBillboardMatrix(m_definition.m_billboardType, g_app->m_worldCamera.GetModelMatrix(), m_position);

	Vec3 viewingDirection = m_position - g_app->m_worldCamera.GetPosition();
	viewingDirection = viewingDirection.GetXY().GetNormalized().ToVec3();
	Mat44 worldToLocal = GetRenderModelMatrix().GetOrthonormalInverse();
	viewingDirection = worldToLocal.TransformVectorQuantity3D(viewingDirection);
	SpriteAnimDefinition animation = m_currentAnimation.GetAnimationForDirection(viewingDirection);
	SpriteDefinition sprite = animation.GetSpriteDefAtTime(m_animationClock.GetTotalSeconds());

	std::vector<Vertex_PCU> unlitVertexes;
	std::vector<Vertex_PCUTBN> litVertexes;

	if (m_definition.m_isLit)
	{
		AddVertsForRoundedQuad3D(litVertexes, Vec3::ZERO, Vec3::NORTH * m_definition.m_size.x, Vec3::NORTH * m_definition.m_size.x + Vec3::SKYWARD * m_definition.m_size.y, Vec3::SKYWARD * m_definition.m_size.y, Rgba8::WHITE, sprite.GetUVs());
		TransformVertexArray3D(litVertexes, Mat44::CreateTranslation3D(-Vec3(0.f, m_definition.m_size.x, m_definition.m_size.y) * Vec3(0.f, m_definition.m_pivot.x, m_definition.m_pivot.y)));
	}
	else
	{
		AddVertsForQuad3D(unlitVertexes, Vec3::ZERO, Vec3::NORTH * m_definition.m_size.x, Vec3::NORTH * m_definition.m_size.x + Vec3::SKYWARD * m_definition.m_size.y, Vec3::SKYWARD * m_definition.m_size.y, Rgba8::WHITE, sprite.GetUVs());
		TransformVertexArray3D(unlitVertexes, Mat44::CreateTranslation3D(-Vec3(0.f, m_definition.m_size.x, m_definition.m_size.y) * Vec3(0.f, m_definition.m_pivot.x, m_definition.m_pivot.y)));
	}

	g_renderer->SetBlendMode(BlendMode::OPAQUE);
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_BACK);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetDepthMode(DepthMode::ENABLED);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->SetModelConstants(billboardMatrix, m_definition.m_texture ? Rgba8::WHITE : Rgba8::MAGENTA);
	g_renderer->BindTexture(m_definition.m_texture);
	g_renderer->BindShader(m_definition.m_shader);
	m_definition.m_isLit ? g_renderer->DrawVertexArray(litVertexes) : g_renderer->DrawVertexArray(unlitVertexes);
}

void Actor::RenderDebug() const
{
	DebugAddWorldWireCylinder(m_position, m_position + Vec3::SKYWARD * m_physicsHeight, m_physicsRadius, 0.f, Rgba8::MAGENTA, Rgba8::MAGENTA, DebugRenderMode::USE_DEPTH);
}

void Actor::TakeDamage(float damage)
{
	if (m_isDead)
	{
		return;
	}

	m_health -= damage;
	if (m_health <= 0.f)
	{
		Die();
	}
	else
	{
		m_hurtSoundPlayback = g_audio->StartSoundAt(m_definition.m_hurtSound, m_position);
	}

	if (!m_definition.m_is3DActor)
	{
		m_currentAnimation = m_definition.GetAnimationGroupByName("Hurt");
		if (m_currentAnimation.m_scaleBySpeed)
		{
			m_animationClock.SetTimeScale(m_velocity.GetLength() / m_definition.m_runSpeed);
		}
		else
		{
			m_animationClock.SetTimeScale(1.f);
		}
		m_animationClock.Reset();
	}
}

void Actor::Die()
{
	if (m_isDead)
	{
		return;
	}

	m_isDead = true;

	m_lifetimeTimer = Stopwatch(&m_map->m_game->m_gameClock, m_definition.m_corpseLifetime);
	m_lifetimeTimer.Start();

	if (m_definition.m_deathSound != MISSING_SOUND_ID)
	{
		g_audio->StartSoundAt(m_definition.m_deathSound, m_position);
	}

	if (m_definition.m_explodeOnDie)
	{
		for (int particleIndex = 0; particleIndex < m_definition.m_explosionParticles; particleIndex++)
		{
			float particleSize = g_RNG->RollRandomFloatInRange(m_definition.m_explosionParticleSize);
			Particle* particle = m_map->SpawnParticle(m_position, particleSize, m_definition.m_explosionParticleColor, m_definition.m_explosionParticleLifetime);
			Vec3 randomDirection = Vec3(g_RNG->RollRandomFloatInRange(-1.f, 1.f), g_RNG->RollRandomFloatInRange(-1.f, 1.f), g_RNG->RollRandomFloatInRange(-1.f, 1.f));
			randomDirection = randomDirection.GetNormalized() * m_definition.m_explosionParticleSpeed;
			particle->AddImpulse(randomDirection);
			
			for (int actorIndex = 0; actorIndex < (int)m_map->m_actors.size(); actorIndex++)
			{
				Actor* actor = m_map->m_actors[actorIndex];
				if (!actor)
				{
					continue;
				}

				if (!IsPointInsideDisc2D(actor->m_position.GetXY(), m_position.GetXY(), m_definition.m_explosionRadius))
				{
					continue;
				}

				if (actor->m_UID != m_ownerUID || actor->m_definition.m_faction == Faction::MARINE)
				{
					Vec3 directionToActor = (actor->m_position - m_position).GetNormalized();
					actor->AddImpulse(m_definition.m_impulseOnExplode * directionToActor);
				}

				if (actor->m_UID != m_ownerUID)
				{
					float damage = g_RNG->RollRandomFloatInRange(m_definition.m_explosionDamage);
					actor->TakeDamage(damage);
					if (actor->m_controller)
					{
						actor->m_controller->DamagedBy(m_map->GetActorByUID(m_ownerUID));
					}
				}
			}

		}
	}

	if (m_definition.m_is3DActor)
	{
		return;
	}

	m_currentAnimation = m_definition.GetAnimationGroupByName("Death");
	if (m_currentAnimation.m_scaleBySpeed)
	{
		m_animationClock.SetTimeScale(m_velocity.GetLength() / m_definition.m_runSpeed);
	}
	else
	{
		m_animationClock.SetTimeScale(1.f);
	}
	m_animationClock.Reset();
}

void Actor::AddForce(Vec3 const& force)
{
	m_acceleration += force;
}

void Actor::AddImpulse(Vec3 const& impulse)
{
	m_velocity += impulse;
}

void Actor::OnCollide(Actor* other)
{
	if (other && !other->m_isDead)
	{
		Vec2 actorAPositionXY = m_position.GetXY();
		Vec2 actorBPositionXY = other->m_position.GetXY();

		if (m_definition.m_collidesWithActors && other->m_definition.m_collidesWithActors)
		{
			PushDiscsOutOfEachOther2D(actorAPositionXY, m_physicsRadius, actorBPositionXY, other->m_physicsRadius);
		}
		else if (!m_definition.m_collidesWithActors)
		{
			PushDiscOutOfFixedDisc2D(actorBPositionXY, other->m_physicsRadius, actorAPositionXY, other->m_physicsRadius);
		}
		else if (!other->m_definition.m_collidesWithActors)
		{
			PushDiscOutOfFixedDisc2D(actorAPositionXY, m_physicsRadius, actorBPositionXY, other->m_physicsRadius);
		}

		m_position = Vec3(actorAPositionXY.x, actorAPositionXY.y, m_position.z);
		other->m_position = Vec3(actorBPositionXY.x, actorBPositionXY.y, other->m_position.z);

		if (m_definition.m_damageOnCollide != FloatRange::ZERO)
		{
			other->TakeDamage(g_RNG->RollRandomFloatInRange(m_definition.m_damageOnCollide));
			Actor* owner = m_map->GetActorByUID(m_ownerUID);
			if (owner)
			{
				other->m_controller->DamagedBy(owner);
			}
		}
		other->AddImpulse(GetForwardNormal().GetXY().ToVec3() * m_definition.m_impulseOnCollide);
	}

	if (m_definition.m_dieOnCollide)
	{
		Die();
	}
}

void Actor::OnCollideWithStatic(StaticActor* staticActor)
{
	Vec2 position2D = m_position.GetXY();
	PushDiscOutOfFixedDisc2D(position2D, m_physicsRadius, staticActor->m_position.GetXY(), staticActor->m_physicsRadius);
	m_position = Vec3(position2D.x, position2D.y, m_position.z);

	if (m_definition.m_dieOnCollide)
	{
		Die();
	}
}

void Actor::OnPossessed(Controller* controller)
{
	m_controller = controller;
	if (!controller->IsPlayer())
	{
		m_aiController = controller;
	}

	if (controller->IsPlayer() && g_openXR && g_openXR->IsInitialized())
	{
		m_leftWeapons[0]->OnEquipped(this);
		m_rightWeapons[0]->OnEquipped(this);
	}
}

void Actor::OnUnpossessed()
{
	if (m_aiController)
	{
		m_controller = m_aiController;
	}
	else
	{
		m_controller = nullptr;
	}
}

void Actor::MoveInDirection(Vec3 const& direction, float speed)
{
	AddForce(direction * (speed * m_definition.m_drag));
}

void Actor::TurnInDirection(float targetOrientation, float maxTurnRate)
{
	float deltaSeconds = m_map->m_game->m_gameClock.GetDeltaSeconds();

	m_orientation.m_yawDegrees = GetTurnedTowardDegrees(m_orientation.m_yawDegrees, targetOrientation, maxTurnRate * deltaSeconds);
}

Weapon const* Actor::GetEquippedWeapon() const
{
	return nullptr;
}

void Actor::EquipWeapon(int weaponIndex)
{
	if (weaponIndex < 0 || weaponIndex >= (int)m_weapons.size())
	{
		return;
	}

	m_equippedWeaponIndex = weaponIndex;
	m_weapons[m_equippedWeaponIndex]->OnEquipped(this);
}

void Actor::EquipNextWeapon()
{
	EquipWeapon(m_equippedWeaponIndex + 1);
}

void Actor::EquipPreviousWeapon()
{
	EquipWeapon(m_equippedWeaponIndex - 1);
}

void Actor::Attack()
{
	if (m_isDead)
	{
		return;
	}

	if (m_equippedWeaponIndex > -1)
	{
		m_weapons[m_equippedWeaponIndex]->Fire();
	}
}

Mat44 const Actor::GetModelMatrix() const
{
	Mat44 modelMatrix = Mat44::CreateTranslation3D(m_position);
	modelMatrix.AppendZRotation(m_orientation.m_yawDegrees);

	return modelMatrix;
}

Mat44 const Actor::GetRenderModelMatrix() const
{
	Mat44 renderModelMatrix = Mat44::CreateTranslation3D(m_position);
	renderModelMatrix.Append(m_orientation.GetAsMatrix_iFwd_jLeft_kUp());

	return renderModelMatrix;
}

Vec3 const Actor::GetForwardNormal() const
{
	return m_orientation.GetAsMatrix_iFwd_jLeft_kUp().GetIBasis3D();
}

Vec3 const Actor::GetLeftNormal() const
{
	return m_orientation.GetAsMatrix_iFwd_jLeft_kUp().GetJBasis3D();
}

Vec3 const Actor::GetUpNormal() const
{
	return m_orientation.GetAsMatrix_iFwd_jLeft_kUp().GetKBasis3D();
}

Vec3 const Actor::GetEyePosition() const
{
	Vec3 eyePosition = m_pivotPosition + GetUpNormal() * (m_definition.m_eyeHeight - m_definition.m_weaponHeight) + GetForwardNormal() * 0.01f;
	if (m_isDead)
	{
		eyePosition = Interpolate(eyePosition, m_position, m_lifetimeTimer.GetElapsedFraction());
	}

	return eyePosition;
}

Vec3 const Actor::GetWeaponPosition() const
{
	return m_pivotPosition + GetForwardNormal() * 0.2f + GetLeftNormal() * 0.02f;
}
