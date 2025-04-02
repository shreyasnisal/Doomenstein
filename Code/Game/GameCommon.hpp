#pragma once

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Models/ModelLoader.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/SimpleTriangleFont.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/RaycastUtils.hpp"

#include "Game/ActorUID.hpp"

class App;

extern App*							g_app;
extern RandomNumberGenerator*		g_RNG;
extern Renderer*					g_renderer;
extern AudioSystem*					g_audio;
extern Window*						g_window;
extern BitmapFont*					g_squirrelFont;
extern ModelLoader*					g_modelLoader;

extern float g_screenSizeX;
extern float g_screenSizeY;

struct DoomRaycastResult : RaycastResult3D
{
public:
	ActorUID m_impactActorUID = ActorUID::INVALID;

public:
	DoomRaycastResult() = default;
	DoomRaycastResult(RaycastResult3D const& raycastResult3D);
};

constexpr float GRAVITY = 100.f;

