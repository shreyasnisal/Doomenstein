#include "Game/GameCommon.hpp"


float g_screenSizeY = 800.f;
float g_screenSizeX = 800.f;

DoomRaycastResult::DoomRaycastResult(RaycastResult3D const& raycastResult3D)
{
	m_didImpact = raycastResult3D.m_didImpact;
	m_impactDistance = raycastResult3D.m_impactDistance;
	m_impactPosition = raycastResult3D.m_impactPosition;
	m_impactNormal = raycastResult3D.m_impactNormal;
}
