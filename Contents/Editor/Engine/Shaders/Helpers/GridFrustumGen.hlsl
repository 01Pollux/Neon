
#include "../Common/Frame.hlsli"
#include "../Common/Utils.hlsli"
#include "../Geometry/Frustum.hlsli"

#ifndef CS_KERNEL_SIZE_X
#define CS_KERNEL_SIZE_X 16
#endif

#ifndef CS_KERNEL_SIZE_Y
#define CS_KERNEL_SIZE_Y 16
#endif


//

struct DispatchConstants
{
	uint2 ThreadCounts;
};

ConstantBuffer<DispatchConstants> c_DispatchConstants : register(b0, space1);

RWStructuredBuffer<Frustum> c_FrustumGrid : register(u0, space1);

//

[numthreads(CS_KERNEL_SIZE_X, CS_KERNEL_SIZE_Y, 1)]
void CS_Main(
	uint3 DTID : SV_DispatchThreadID)
{
	[branch]
	if (!(DTID.x < c_DispatchConstants.ThreadCounts.x &&
		  DTID.y < c_DispatchConstants.ThreadCounts.y))
	{
		return;
	}
	
	float3 ViewSpace[4];
	
	// Top left
	ViewSpace[0] = ScreenToPosition(float4(DTID.x * CS_KERNEL_SIZE_X, DTID.y * CS_KERNEL_SIZE_Y, -1.0f, 1.0f), g_FrameData.ScreenResolution, g_FrameData.ProjectionInverse).xyz;
	// Top right
	ViewSpace[1] = ScreenToPosition(float4((DTID.x + 1) * CS_KERNEL_SIZE_X, DTID.y * CS_KERNEL_SIZE_Y, -1.0f, 1.0f), g_FrameData.ScreenResolution, g_FrameData.ProjectionInverse).xyz;
	// Bottom left
	ViewSpace[2] = ScreenToPosition(float4(DTID.x * CS_KERNEL_SIZE_X, (DTID.y + 1) * CS_KERNEL_SIZE_Y, -1.0f, 1.0f), g_FrameData.ScreenResolution, g_FrameData.ProjectionInverse).xyz;
	// Bottom right
	ViewSpace[3] = ScreenToPosition(float4((DTID.x + 1) * CS_KERNEL_SIZE_X, (DTID.y + 1) * CS_KERNEL_SIZE_Y, -1.0f, 1.0f), g_FrameData.ScreenResolution, g_FrameData.ProjectionInverse).xyz;

	float3 EyePosition = (float3) 0;

	Frustum Frustum;
	Frustum.planes[0] = PlaneFromPoints(EyePosition, ViewSpace[2], ViewSpace[0]);
	Frustum.planes[1] = PlaneFromPoints(EyePosition, ViewSpace[1], ViewSpace[3]);
	Frustum.planes[2] = PlaneFromPoints(EyePosition, ViewSpace[0], ViewSpace[1]);
	Frustum.planes[3] = PlaneFromPoints(EyePosition, ViewSpace[3], ViewSpace[2]);

	Frustum.planes[0].Normal = normalize(mul(float4(Frustum.planes[0].Normal, 0.f), g_FrameData.ViewInverse).xyz);
	Frustum.planes[1].Normal = normalize(mul(float4(Frustum.planes[1].Normal, 0.f), g_FrameData.ViewInverse).xyz);
	Frustum.planes[2].Normal = normalize(mul(float4(Frustum.planes[2].Normal, 0.f), g_FrameData.ViewInverse).xyz);
	Frustum.planes[3].Normal = normalize(mul(float4(Frustum.planes[3].Normal, 0.f), g_FrameData.ViewInverse).xyz);

	uint Index = DTID.y * c_DispatchConstants.ThreadCounts.x + DTID.x;
	c_FrustumGrid[Index] = Frustum;
}