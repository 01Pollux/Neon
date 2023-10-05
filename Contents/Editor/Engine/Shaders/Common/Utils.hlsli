#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

//

// Convert clip space coordinates to matrix space
float4 ClipToViewPosition(
	in const float4 ClipPos,
	in const matrix MatrixInverse)
{
	float4 WorldPos = mul(ClipPos, MatrixInverse);
	return WorldPos / WorldPos.w;
}

// Convert screen space coordinates to matrix space
float4 ScreenToPosition(
	in const float4 ScreenPos,
	in const float2 ScreenSize,
	in const matrix MatrixInverse)
{
	float2 ScreenUV = ScreenPos.xy / ScreenSize;
	ScreenUV.y = 1.0f - ScreenUV.y;
	ScreenUV = ScreenUV * 2.0f - 1.0f;
	
	return ClipToViewPosition(float4(ScreenUV, ScreenPos.z, 1.0f), MatrixInverse);
}

//

float3 UVToViewPosition(
	in const float2 UV,
	in const float Depth,
	in const matrix ProjectionInverse)
{
	float2 ScreenUV = UV * 2.0f - 1.0f;
	ScreenUV.y *= -1.0f;
	
	float4 ScreenPos = float4(ScreenUV, Depth, 1.0f);
	float4 WorldPos = mul(ScreenPos, ProjectionInverse);
	
	return WorldPos.xyz / WorldPos.w;
}

float3 UVToWorldPosition(
	in const float2 UV,
	in const float Depth,
	in const matrix ProjectionInverse)
{
	float2 ScreenUV = UV * 2.0f - 1.0f;
	ScreenUV.y *= -1.0f;
	
	float4 ScreenPos = float4(ScreenUV, Depth, 1.0f);
	float4 WorldPos = mul(ScreenPos, ProjectionInverse);
	
	return WorldPos.xyz / WorldPos.w;
}


//

SamplerState s_Sampler_PointWrap : register(s0, space0);
SamplerState s_Sampler_PointClamp : register(s1, space0);
SamplerState s_Sampler_LinearWrap : register(s2, space0);
SamplerState s_Sampler_LinearClamp : register(s3, space0);
SamplerState s_Sampler_LinearBorder : register(s4, space0);
SamplerState s_Sampler_AnisotropicWrap : register(s5, space0);
SamplerState s_Sampler_AnisotropicClamp : register(s6, space0);

#endif