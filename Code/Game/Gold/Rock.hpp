#pragma once

#include "Game/Gold/StaticActor.hpp"

#include "Engine/Core/Models/Model.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Mat44.hpp"

#include <string>
#include <vector>

class Rock : public StaticActor
{
public:
	~Rock() = default;
	Rock(Map* map, Vec3 const& position, EulerAngles const& orientation, float scale = 1.f, Rgba8 const& tint = Rgba8::WHITE);

	virtual void Render() const override;
	void LoadModel();

public:
	Model* m_model = nullptr;
	Mat44 m_transform = Mat44::IDENTITY;
	Rgba8 m_tint = Rgba8::WHITE;
};
