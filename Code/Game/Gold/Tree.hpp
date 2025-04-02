#pragma once

#include "Game/Gold/StaticActor.hpp"

#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Models/Model.hpp"

#include <string>

class Tree : public StaticActor
{
public:
	~Tree() = default;
	Tree(Map* map, Vec3 const& m_position, float scale = 1.f);

	virtual void Render() const override;

public:
	Model* m_model = nullptr;
};
