
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
	float4 Samples[SSAO_SAMPLE_COUNT];
	
	float Radius;
	float Bias;
	float Magnitude;
	float Contrast;
	float ResolutionFactor;
};

//

ConstantBuffer<SSAOParams> c_SSAOParams : register(b1, space0);

Texture2D<float4> c_NormalMap : register(t0, space0);
Texture2D<float> c_DepthMap : register(t1, space0);
Texture2D<float3> c_NoiseMap : register(t2, space0);

RWTexture2D<float> c_OcclusionOutput : register(u0, space0);

// --------------------
// Compute Shader
// --------------------

[numthreads(CS_KERNEL_SIZE_X, CS_KERNEL_SIZE_Y, CS_KERNEL_SIZE_Z)]
void CS_Main(
	uint3 DTID : SV_DispatchThreadID)
{
	float2 Resolution = g_FrameData.ScreenResolution * c_SSAOParams.ResolutionFactor;
	float2 UV = DTID.xy / Resolution;
	
	float3 Normal = c_NormalMap.SampleLevel(s_Sampler_LinearClamp, UV, 0.f).xyz;
	Normal = Normal * 2.f - 1.f;
	Normal = normalize(Normal);
	
	float Depth = c_DepthMap.SampleLevel(s_Sampler_LinearClamp, UV, 0.f).x;
	float3 Position = UVToViewPosition(UV, Depth, g_FrameData.ProjectionInverse);
	
	float3 Random = normalize(c_NoiseMap.SampleLevel(s_Sampler_LinearWrap, UV, 0.f).xyz * 2.f - 1.f);
	
	float3 Tangent = normalize(Random - Normal * dot(Random, Normal));
	float3 Bitangent = cross(Normal, Tangent);
	float3x3 TBN = float3x3(Tangent, Bitangent, Normal);

	float Occlusion = (float) SSAO_SAMPLE_COUNT;

	[unroll(SSAO_SAMPLE_COUNT)]
	for (int i = 0; i < SSAO_SAMPLE_COUNT; ++i)
	{
		float3 SamplePos = mul(c_SSAOParams.Samples[i].xyz, TBN);
		SamplePos = Position + SamplePos * c_SSAOParams.Radius;
		
		float3 SampleUV = mul(float4(SamplePos, 1.f), g_FrameData.ViewProjection).xyw;
		SampleUV.xy = ((SampleUV.xy / SampleUV.z) * .5f * float2(1.f, -1.f) + .5f);
		
		float SampleDepth = c_DepthMap.SampleLevel(s_Sampler_LinearClamp, SampleUV.xy, 0.f);
		SampleDepth = UVToViewPosition(SampleUV.xy, SampleDepth, g_FrameData.ProjectionInverse).z;
		
		float Occluded = step(SampleDepth + c_SSAOParams.Bias, SamplePos.z);
		float Intensity = smoothstep(.0f, 1.f, c_SSAOParams.Radius * (Position.z - SampleDepth));
		
		Occlusion -= Occluded * Intensity;
	}
	
	Occlusion /= SSAO_SAMPLE_COUNT;
	Occlusion = pow(Occlusion, c_SSAOParams.Magnitude);
	Occlusion = c_SSAOParams.Contrast * (Occlusion - .5f) + .5f;
	
	c_OcclusionOutput[DTID.xy] = Occlusion;
}