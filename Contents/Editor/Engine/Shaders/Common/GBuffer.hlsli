#ifndef GBUFFER_HELPER_H
#define GBUFFER_HELPER_H

struct PSOutput
{
	float4 Albedo_Roughness : SV_TARGET0;
	float4 Normal_Metallic : SV_TARGET1;
	float4 Emissive : SV_TARGET2;
};

PSOutput GBufferPack(
	float3 Albedo,
	float Roughness,
	float3 Normal,
	float Metallic,
	float4 Emissive)
{
	PSOutput Out;
	Out.Albedo_Roughness = float4(Albedo, Roughness);
	Out.Normal_Metallic = float4(Normal * 0.5f + 0.5f, Metallic);
	Out.Emissive = Emissive;
	return Out;
}

#endif