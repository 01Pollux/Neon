#ifndef GEOMETRY_FRUSTUM_H
#define GEOMETRY_FRUSTUM_H


#include "Shapes.hlsli"

struct Frustum
{
	Plane planes[4]; // left, right, top, bottom
};

//

Frustum CreateFrustum(in float fov, in float aspect, in float nearZ, in float farZ)
{
	Frustum frustum;
	float tanFov = tan(fov * 0.5f);
	float nearY = nearZ * tanFov;
	float nearX = nearY * aspect;
	float farY = farZ * tanFov;
	float farX = farY * aspect;

	frustum.planes[0] = PlaneFromPoints(float3(0.0f, 0.0f, 0.0f), float3(1.0f, 0.0f, 1.0f), nearX);
	frustum.planes[1] = PlaneFromPoints(float3(0.0f, 0.0f, 0.0f), float3(-1.0f, 0.0f, 1.0f), nearX);
	frustum.planes[2] = PlaneFromPoints(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 1.0f), nearY);
	frustum.planes[3] = PlaneFromPoints(float3(0.0f, 0.0f, 0.0f), float3(0.0f, -1.0f, 1.0f), nearY);
	return frustum;
}

bool IsInsideFrustum(const in Frustum frustum, const in float3 pt, const in float3 radius, out float3 outPt, out float outDist, out int outPlane)
{
	outPt = pt;
	outDist = 0.0f;
	for (int i = 0; i < 6; ++i)
	{
		float dist = dot(frustum.planes[i].Normal, pt) + frustum.planes[i].Distance;
		if (dist < -radius[i])
		{
			outPt += frustum.planes[i].Normal * (dist + radius[i]);
			outDist = dist + radius[i];
			outPlane = i;
			return false;
		}
	}
	return true;
}

bool IsInsideFrustum(const in Frustum frustum, const in float3 pt, const in float radius, out float3 outPt, out float outDist, out int outPlane)
{
	outPt = pt;
	outDist = 0.0f;
	for (int i = 0; i < 6; ++i)
	{
		float dist = dot(frustum.planes[i].Normal, pt) + frustum.planes[i].Distance;
		if (dist < -radius)
		{
			outPt += frustum.planes[i].Normal * (dist + radius);
			outDist = dist + radius;
			outPlane = i;
			return false;
		}
	}
	return true;
}
	
#endif