
#include "../Common/Frame.hlsli"
#include "../Common/Utils.hlsli"
#include "../Common/Light.hlsli"
#include "../Geometry/Frustum.hlsli"

#define CS_KERNEL_SIZE_X LIGHT_CLUSTER_SIZE
#define CS_KERNEL_SIZE_Y LIGHT_CLUSTER_SIZE

//

struct LightInfo
{
	uint LightCount;
};

//

ConstantBuffer<LightInfo> c_LightInfo : register(b0, space1);

Texture2D<float> c_DepthBuffer : register(t0, space1);
StructuredBuffer<Frustum> c_FrustumGrid : register(t1, space1);
StructuredBuffer<Light> c_Lights : register(t2, space1);

// First index is the number of lights
RWByteAddressBuffer c_LightIndexList_Counters : register(u0, space1);
RWStructuredBuffer<uint> c_LightIndexList_Opaque : register(u1, space1);
RWStructuredBuffer<uint> c_LightIndexList_Transparent : register(u2, space1);

RWTexture2D<uint2> c_LightGrid_Opaque : register(u3, space1);
RWTexture2D<uint2> c_LightGrid_Transparent : register(u4, space1);

//

groupshared uint c_MinDepth;
groupshared uint c_MaxDepth;
groupshared Frustum c_GroupFrustum;

// Opaque geometry light lists.
groupshared uint c_LightCount_Opaque;
groupshared uint c_LightIndexStartOffset_Opaque;
groupshared uint c_LightList_Opaque[MAX_LIGHTS];

// Transparent geometry light lists.
groupshared uint c_LightCount_Transparent;
groupshared uint c_LightIndexStartOffset_Transparent;
groupshared uint c_LightList_Transparent[MAX_LIGHTS];

//

void AppendLight_Opaque(in const uint Index)
{
	int InsertIndex;
	InterlockedAdd(c_LightCount_Opaque, 1, InsertIndex);
	if (InsertIndex < MAX_LIGHTS)
	{
		c_LightList_Opaque[InsertIndex] = Index;
	}
}

void AppendLight_Transparent(in const uint Index)
{
	int InsertIndex;
	InterlockedAdd(c_LightCount_Transparent, 1, InsertIndex);
	if (InsertIndex < MAX_LIGHTS)
	{
		c_LightList_Transparent[InsertIndex] = Index;
	}
}

//

[numthreads(CS_KERNEL_SIZE_X, CS_KERNEL_SIZE_Y, 1)]
void CS_Main(
	uint GID : SV_GroupIndex,
	uint3 GTID : SV_GroupID,
	uint3 DTID : SV_DispatchThreadID)
{
	int2 TexCoord = DTID.xy;
	float Depth = c_DepthBuffer[TexCoord];
	
	uint DepthUint = asuint(Depth);
	if (GID == 0)
	{
		uint GroupCount = g_FrameData.ScreenResolution.x / CS_KERNEL_SIZE_X;
		
		c_MinDepth = 0xFFFFFFFF;
		c_MaxDepth = 0;
		c_LightCount_Opaque = 0;
		c_LightCount_Transparent = 0;
		
		uint Index = (GTID.y * GroupCount) + GTID.x;
		c_GroupFrustum = c_FrustumGrid[Index];
	}

	GroupMemoryBarrierWithGroupSync();
	
	InterlockedMin(c_MinDepth, Depth);
	InterlockedMax(c_MaxDepth, Depth);
	
	GroupMemoryBarrierWithGroupSync();
	
	float MinDepth = asfloat(c_MinDepth);
	float MaxDepth = asfloat(c_MaxDepth);
	
	float MinDepthWS = ScreenToPosition(float4(0.f, 0.f, MinDepth, 1.f), g_FrameData.ScreenResolution, g_FrameData.ViewProjectionInverse).z;
	float MaxDepthWS = ScreenToPosition(float4(0.f, 0.f, MaxDepth, 1.f), g_FrameData.ScreenResolution, g_FrameData.ViewProjectionInverse).z;
	float NearClipWS = ScreenToPosition(float4(0.f, 0.f, 0.f, 1.f), g_FrameData.ScreenResolution, g_FrameData.ViewProjectionInverse).z;
	
	Plane MinPlane = { float3(0.f, 0.f, -1.f), -MinDepthWS };

    // Each thread in a group will cull 1 light until all lights have been culled.
	uint i = GID;
	while (i < c_LightInfo.LightCount)
	{
		Light CurLight = c_Lights[i];
		if (CurLight.IsEnabled())
		{
			i += CS_KERNEL_SIZE_X * CS_KERNEL_SIZE_Y;
			switch (CurLight.GetType())
			{
				case LIGHT_FLAGS_TYPE_DIRECTIONAL:
					{
						AppendLight_Opaque(i);
						AppendLight_Transparent(i);
						break;
					}
				case LIGHT_FLAGS_TYPE_POINT:
					{
						Sphere LightSphere = { CurLight.Position, CurLight.Range };
						if (SphereInsideFrustum(c_GroupFrustum, LightSphere, MinDepthWS, MaxDepthWS))
						{
							AppendLight_Transparent(i);
							if (SphereInsidePlane(LightSphere, MinPlane))
							{
								AppendLight_Opaque(i);
							}
						}
						break;
					}
				case LIGHT_FLAGS_TYPE_SPOT_LIGHT:
					{
						float Radius = tan(radians(CurLight.Spot_GetAngle())) * CurLight.Range;
						Cone LightCone = { CurLight.Position, CurLight.Range, CurLight.Direction, Radius };
						if (ConeInsideFrustum(c_GroupFrustum, LightCone, MinDepthWS, MaxDepthWS))
						{
							AppendLight_Transparent(i);
							if (ConeInsidePlane(LightCone, MinPlane))
							{
								AppendLight_Opaque(i);
							}
						}
						break;
					}
			}
		}
	}

	GroupMemoryBarrierWithGroupSync();
	
	if (GID == 0)
	{
		c_LightIndexList_Counters.InterlockedAdd(0, c_LightCount_Opaque, c_LightIndexStartOffset_Opaque);
		c_LightGrid_Opaque[GTID.xy] = uint2(c_LightIndexStartOffset_Opaque, c_LightCount_Opaque);

		c_LightIndexList_Counters.InterlockedAdd(4, c_LightCount_Transparent, c_LightIndexStartOffset_Transparent);
		c_LightGrid_Transparent[GTID.xy] = uint2(c_LightIndexStartOffset_Transparent, c_LightCount_Transparent);
	}
	
	GroupMemoryBarrierWithGroupSync();
	
	for (i = 0; i < c_LightCount_Opaque; i += CS_KERNEL_SIZE_X * CS_KERNEL_SIZE_Y)
	{
		c_LightIndexList_Opaque[c_LightIndexStartOffset_Opaque + i] = c_LightList_Opaque[i];
	}
	for (i = 0; i < c_LightCount_Transparent; i += CS_KERNEL_SIZE_X * CS_KERNEL_SIZE_Y)
	{
		c_LightIndexList_Transparent[c_LightIndexStartOffset_Transparent + i] = c_LightList_Transparent[i];
	}
}