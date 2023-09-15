
#include "../Common/Frame.hlsli"
#include "../Common/Utils.hlsli"

#ifndef CS_KERNEL_SIZE_X
#define CS_KERNEL_SIZE_X 16
#endif

#ifndef CS_KERNEL_SIZE_Y
#define CS_KERNEL_SIZE_Y 16
#endif

#ifndef CS_KERNEL_SIZE_Z
#define CS_KERNEL_SIZE_Z 1
#endif

#ifndef SSAO_SAMPLE_COUNT
#define SSAO_SAMPLE_COUNT 16
#endif

//

struct SSAOParams
{
	float Radius;
	float Bias;
	float Magnitude;
	float Contrast;
	float ResolutionFactor;
	
	float4 Samples[SSAO_SAMPLE_COUNT];
};

enum
{
	TEXTURE_MAP_NORMAL,
	TEXTURE_MAP_DEPTH,
	TEXTURE_MAP_NOISE,
};

//

ConstantBuffer<SSAOParams> c_SSAOParams : register(b0, space1);

Texture2D c_TextureMap[] : register(t0, space1);

RWTexture2D<float> c_OcclusionOutput : register(u0, space1);

// --------------------
// Compute Shader
// --------------------

[numthreads(CS_KERNEL_SIZE_X, CS_KERNEL_SIZE_Y, CS_KERNEL_SIZE_Z)]
void CS_Main(
	uint3 DTID : SV_DispatchThreadID)
{
	Texture2D NormalMap = c_TextureMap[TEXTURE_MAP_NORMAL];
	Texture2D DepthMap = c_TextureMap[TEXTURE_MAP_DEPTH];
	Texture2D NoiseMap = c_TextureMap[TEXTURE_MAP_NOISE];
	
	float2 Resolution = g_FrameData.ScreenResolution * c_SSAOParams.ResolutionFactor;
	float2 UV = ((float2) DTID.xy + 0.5f) / Resolution;
	
	float3 Normal = NormalMap.SampleLevel(s_Sampler_LinearClamp, UV, 0.f).xyz;
	Normal = Normal * 2.f - 1.f;
	Normal = normalize(Normal);
	
	float Depth = DepthMap.SampleLevel(s_Sampler_LinearClamp, UV, 0.f).x;
	float3 Position = UVToViewPosition(UV, Depth, g_FrameData.ProjectionInverse);
	
	float3 Random = normalize(NoiseMap.SampleLevel(s_Sampler_LinearWrap, UV, 0.f).xyz * 2.f - 1.f);
	
	float3 Tangent = normalize(Random - Normal * dot(Random, Normal));
	float3 Bitangent = cross(Normal, Tangent);
	float3x3 TBN = float3x3(Tangent, Bitangent, Normal);

	float Occlusion = (float) SSAO_SAMPLE_COUNT;

	[unroll(SSAO_SAMPLE_COUNT)]
	for (int i = 0; i < SSAO_SAMPLE_COUNT; ++i)
	{
		float3 SampleDir = mul(c_SSAOParams.Samples[i].xyz, TBN);
		float3 SamplePos = Position + SampleDir * c_SSAOParams.Radius;
		
		float4 Offset = float4(SamplePos, 1.f);
		Offset = mul(Offset, g_FrameData.Projection);
		Offset.xy = ((Offset.xy / Offset.w) * float2(1.0f, -1.0f)) * 0.5f + 0.5f;
		
		float SampleDepth = DepthMap.SampleLevel(s_Sampler_LinearClamp, Offset.xy, 0.f).x;
		SampleDepth = UVToViewPosition(Offset.xy, SampleDepth, g_FrameData.ProjectionInverse).z;
		
		float Occluded = step(SampleDepth + c_SSAOParams.Bias, SamplePos.z);
		float Intensity = smoothstep(.0f, 1.f, c_SSAOParams.Radius * (Position.z - SampleDepth));
		
		Occlusion -= Occluded * Intensity;
	}
	
	Occlusion /= SSAO_SAMPLE_COUNT;
	Occlusion = pow(Occlusion, c_SSAOParams.Magnitude);
	Occlusion = c_SSAOParams.Contrast * (Occlusion - .5f) + .5f;
	
	c_OcclusionOutput[DTID.xy] = Occlusion;
}