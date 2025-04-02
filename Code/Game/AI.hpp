#pragma once

#include "Game/Controller.hpp"

class AI : public Controller
{
public:
	~AI() = default;
	AI() = default;

	virtual void Update() override;

	virtual bool IsPlayer() const override { return false; }
	virtual void DamagedBy(Actor * actor) override;
	virtual void KilledBy(Actor * actor) override;
	virtual void Killed(Actor * actor) override;

public:
	ActorUID m_targetUID = ActorUID::INVALID;
};
