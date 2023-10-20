#ifndef GEOMETRY_FRUSTUM_H
#define GEOMETRY_FRUSTUM_H

#include "Shapes.hlsli"
#include "Quaternion.hlsli"

struct Frustum
{
	float4 Orientation;
	float3 Origin;

	float RightSlope;
	float LeftSlope;
	float TopSlope;
	float BottomSlope;
	float Near;
	float Far;
	
	void Init()
	{
		Origin = float3(0.f, 0.f, 0.f);
		Orientation = QUATERNION_IDENTITY;

		RightSlope = 1.f;
		LeftSlope = -1.f;
		TopSlope = 1.f;
		BottomSlope = -1.f;
		
		Near = 0.f;
		Far = 1.f;
	}
	
	void Init(const in float4x4 InvProjection)
	{
		float4 HomogenousPoints[6] =
		{
			float4(1.0f, 0.0f, 1.0f, 1.0f), // right (at far plane)
			float4(-1.0f, 0.0f, 1.0f, 1.0f), // left
			float4(0.0f, 1.0f, 1.0f, 1.0f), // top
			float4(0.0f, -1.0f, 1.0f, 1.0f), // bottom
			float4(0.0f, 0.0f, 0.0f, 1.0f), // near
			float4(0.0f, 0.0f, 1.0f, 1.0f) // far
		};

		// Compute the frustum corners in world space.
		float4 Points[6];
		[unroll(6)]
		for (int i = 0; i < 6; i++)
			Points[i] = mul(HomogenousPoints[i], InvProjection);
		
		Origin = float3(0.0f, 0.0f, 0.0f);
		Orientation = QUATERNION_IDENTITY;

		// Compute the slopes.
		RightSlope = Points[0].x / Points[0].
z;
		LeftSlope = Points[1].x / Points[1].z;
		TopSlope = Points[2].y / Points[2].
z;
		BottomSlope = Points[3].y / Points[3].z;

		// Compute near and far.
		Near = Points[4].z / Points[4].
w;
		Far = Points[5].z / Points[5].w;
	}
	
	void GetPlanes(out float4 Planes[6])
	{
		Planes[0] = float4(0.0f, 0.0f, -1.0f, Near);
		Planes[1] = float4(0.0f, 0.0f, 1.0f, -Far);
		Planes[2] = float4(1.0f, 0.0f, -RightSlope, 0.0f);
		Planes[3] = float4(-1.0f, 0.0f, LeftSlope, 0.0f);
		Planes[4] = float4(0.0f, 1.0f, -TopSlope, 0.0f);
		Planes[5] = float4(0.0f, -1.0f, BottomSlope, 0.0f);
	}
};

//

bool PointInsideFrustum(const in Frustum frustum, const in float3 pt)
{
	float4 Planes[6];
	frustum.GetPlanes(Planes);

    // Transform point into local space of frustum.
	float4 TPoint = float4(q_rotate_vector(pt - frustum.Origin, frustum.Orientation), 1.f);

    // Test point against each plane of the frustum.
	for (int i = 0; i < 6; ++i)
	{
		if (dot(TPoint, Planes[i]) > 0.f)
		{
			return false;
		}
	}

	return true;
}

//

bool SphereInsideFrustum(const in Frustum frustum, const in Sphere sphere)
{
	return true;
}

bool ConeInsideFrustum(const in Frustum frustum, const in Cone cone)
{
	return true;
}
	
#endif