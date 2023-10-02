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

bool PointInsideFrustum(const in Frustum frustum, const in float3 pt, const in float near, const in float far)
{
	bool result = true;
	if ((pt.z > near) || (pt.z < far))
	{
		result = false;
	}

	for (int i = 0; i < 4 && result; i++)
	{
		if (PointInsidePlane(frustum.planes[i], pt))
		{
			result = false;
		}
	}

	return result;
}

//

bool SphereInsideFrustum(const in Frustum frustum, const in Sphere sphere, const in float near, const in float far)
{
	bool result = true;
	if (((sphere.Center.z - sphere.Radius) > near) || ((sphere.Center.z + sphere.Radius) < far))
	{
		result = false;
	}

	for (int i = 0; i < 4 && result; i++)
	{
		if (SphereInsidePlane(sphere, frustum.planes[i]))
		{
			result = false;
		}
	}

	return result;
}

bool ConeInsideFrustum(const in Frustum frustum, const in Cone cone, const in float near, const in float far)
{
	bool result = true;

	Plane nearPlane = { float3(0, 0, -1), -near };
	Plane farPlane = { float3(0, 0, 1), far };

    // First check the near and far clipping planes.
	if (!ConeInsidePlane(cone, nearPlane) && !ConeInsidePlane(cone, farPlane))
	{
		// Then check frustum planes
		for (int i = 0; i < 4; i++)
		{
			if (ConeInsidePlane(cone, frustum.planes[i]))
			{
				result = false;
				break;
			}
		}
	}

	return result;
}
	
#endif