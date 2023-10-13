#ifndef COMMON_STD_MATERIAL_H
#define COMMON_STD_MATERIAL_H

#include "Material.hlsli"

struct StdMaterialData
{
	float3 Albedo;
	uint AlbedoMapIndex;
	
	float3 Specular;
	uint SpecularMapIndex;
	
	float3 Emissive;
	uint EmissiveMapIndex;
	
	uint NormalMapIndex;
};

#endif