#pragma once

#include "Game/Actor.hpp"
#include "Game/ActorUID.hpp"
#include "Game/Gold/StaticActor.hpp"

#include "Engine/Core/Models/Model.hpp"

class Map;

class PlayerActor : public Actor
{
public:
	~PlayerActor() = default;
	PlayerActor(Map* map, ActorUID const& uid, Vec3 const& position, EulerAngles const& orientation);

	virtual Vec3 const GetEyePosition() const override;
	virtual void Render() const override;

public:
	Model* m_model = nullptr;
};
