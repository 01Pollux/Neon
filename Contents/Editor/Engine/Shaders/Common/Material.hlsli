#ifndef COMMON_MATERIAL_H
#define COMMON_MATERIAL_H

#include "../Common/Frame.hlsli"
#include "../Common/Utils.hlsli"
#include "../Common/Light.hlsli"
#include "../Common/Standards.hlsli"

//

StructuredBuffer<Light> _Lights : register(t0, space1);
StructuredBuffer<uint> _LightIndexList : register(t1, space1);
Texture2D<uint2> _LightGrid : register(t2, space1);

//

struct PerInstanceData
{
	matrix World;
};

StructuredBuffer<PerInstanceData> g_InstanceData : register(t0, space2);

//

#define NEON_MATERIAL_SHARED(StructName) ConstantBuffer<StructName> g_Shared : register(b0, space47)
#define NEON_MATERIAL_LOCAL(StructName) StructuredBuffer<StructName> g_Local : register(t0, space47)

#endif