#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

float3 UVToViewPosition(
	float2 UV,
	float Depth,
	matrix ProjectionInverse)
{
	float2 ScreenUV = UV * 2.0f - 1.0f;
	ScreenUV.y *= -1.0f;
	
	float4 ScreenPos = float4(ScreenUV, Depth, 1.0f);
	float4 WorldPos = mul(ScreenPos, ProjectionInverse);
	
	return WorldPos.xyz / WorldPos.w;
}

float3 UVToWorldPosition(
	float2 UV,
	float Depth,
	matrix ProjectionInverse)
{
	float2 ScreenUV = UV * 2.0f - 1.0f;
	ScreenUV.y *= -1.0f;
	
	float4 ScreenPos = float4(ScreenUV, Depth, 1.0f);
	float4 WorldPos = mul(ScreenPos, ProjectionInverse);
	
	return WorldPos.xyz / WorldPos.w;
}

SamplerState s_Sampler_PointWrap : register(s0, space0);
SamplerState s_Sampler_PointClamp : register(s1, space0);
SamplerState s_Sampler_LinearWrap : register(s2, space0);
SamplerState s_Sampler_LinearClamp : register(s3, space0);
SamplerState s_Sampler_LinearBorder : register(s4, space0);
SamplerState s_Sampler_AnisotropicWrap : register(s5, space0);
SamplerState s_Sampler_AnisotropicClamp : register(s6, space0);

#endif