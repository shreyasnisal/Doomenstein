#include "Game/AI.hpp"

#include "Game/Map.hpp"
#include "Game/Actor.hpp"
#include "Game/Weapon.hpp"

void AI::Update()
{
	Actor* possessedActor = m_map->GetActorByUID(m_actorUID);

	if (!possessedActor)
	{
		return;
	}

	if (m_targetUID == ActorUID::INVALID || !m_map->GetActorByUID(m_targetUID))
	{
		Actor* target = m_map->GetClosestVisibleEnemy(possessedActor);
		if (target)
		{
			m_targetUID = target->m_UID;
			g_audio->StartSoundAt(possessedActor->m_definition.m_seeSound, possessedActor->m_position);
		}
	}

	if (m_targetUID != ActorUID::INVALID)
	{
		Actor* target = m_map->GetActorByUID(m_targetUID);
		if (!target)
		{
			return;
		}
		if (!m_map->IsActorAlive(target))
		{
			return;
		}

		Vec2 displacement2DTowardsTarget = target->m_position.GetXY() - possessedActor->m_position.GetXY();
		Vec3 firePosition = possessedActor->GetEyePosition() + possessedActor->GetForwardNormal() * possessedActor->m_physicsRadius;
		Vec2 displacementFirePositionToTarget = target->m_position.GetXY() - firePosition.GetXY();

		float movementSpeed = possessedActor->m_definition.m_runSpeed;

		if (displacement2DTowardsTarget.GetLengthSquared() < 4.f)
		{
			movementSpeed = possessedActor->m_definition.m_walkSpeed;
		}

		bool isTargetWithinFiringRange = displacementFirePositionToTarget.GetLengthSquared() < possessedActor->m_weapons[possessedActor->m_equippedWeaponIndex]->GetRange() * possessedActor->m_weapons[possessedActor->m_equippedWeaponIndex]->GetRange();
		bool isTargetWithinLOS = m_map->HasLineOfSight(possessedActor, target);
		bool isTargetWithinFiringAngle = fabsf(GetAngleDegreesBetweenVectors2D(displacementFirePositionToTarget, possessedActor->GetForwardNormal().GetXY())) < 15.f;

		if (isTargetWithinFiringRange && isTargetWithinLOS && isTargetWithinFiringAngle)
		{
			possessedActor->Attack();
		}
		else
		{
			possessedActor->MoveInDirection(possessedActor->GetForwardNormal(), movementSpeed);
		}
		possessedActor->TurnInDirection(displacement2DTowardsTarget.GetOrientationDegrees(), possessedActor->m_definition.m_turnSpeed);
	}
}

void AI::DamagedBy(Actor* actor)
{
	m_targetUID = actor->m_UID;
}

void AI::KilledBy(Actor* actor)
{
	UNUSED(actor);
}

void AI::Killed(Actor* actor)
{
	UNUSED(actor);
}
