#pragma once

#include "Game/Actor.hpp"

#include "Engine/Core/Models/Model.hpp"

class Dragon : public Actor
{
public:
	~Dragon() = default;
	Dragon(Map* map, SpawnInfo const& spawnInfo, ActorUID uid);

	virtual void				Render() const override;

public:
	Model* m_model = nullptr;
	Mat44 m_transform = Mat44::IDENTITY;
};
