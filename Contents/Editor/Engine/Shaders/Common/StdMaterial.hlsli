#ifndef COMMON_STD_MATERIAL_H
#define COMMON_STD_MATERIAL_H

#include "Material.hlsli"

struct StdMaterialData
{
	uint _Tex2D_AlbedoIndex;
	uint _Tex2D_SpecularIndex;
	uint _Tex2D_EmissiveIndex;
	uint _Tex2D_NormalIndex;
	
	float3 Color_Albedo;
	float3 Color_Specular;
	float3 Color_Emissive;
};

#endif