#pragma once

#include "Game/ActorUID.hpp"

class Map;

class Controller
{
	friend class Actor;

public:
	virtual ~Controller() = default;
	Controller() = default;

	virtual void Update() = 0;
	
	Actor* GetActor() const;
	virtual void Possess(Actor* actor);
	virtual void Unpossess(Actor* actor);

	virtual bool IsPlayer() const = 0;
	virtual void DamagedBy(Actor* actor) = 0;
	virtual void KilledBy(Actor* actor) = 0;
	virtual void Killed(Actor* actor) = 0;

public:
	ActorUID m_actorUID = ActorUID::INVALID;
	Map* m_map = nullptr;

};
