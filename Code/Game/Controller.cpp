#include "Game/Controller.hpp"

#include "Game/Actor.hpp"
#include "Game/Map.hpp"


void Controller::Possess(Actor* actor)
{
	m_actorUID = actor->m_UID;
	actor->OnPossessed(this);
}

void Controller::Unpossess(Actor* actor)
{
	m_actorUID = ActorUID::INVALID;
	actor->OnUnpossessed();
}

Actor* Controller::GetActor() const
{
	if (!m_map)
	{
		return nullptr;
	}

	return m_map->GetActorByUID(m_actorUID);
}
