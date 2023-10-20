
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
	
	Frustum frustum;
	frustum.Init(g_FrameData.ProjectionInverse);
	
	uint Index = DTID.y * c_DispatchConstants.ThreadCounts.x + DTID.x;
	c_FrustumGrid[Index] = frustum;
}