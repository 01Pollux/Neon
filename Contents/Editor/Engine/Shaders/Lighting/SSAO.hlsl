
#include "../Common/Frame.hlsli"
#include "../Common/Utils.hlsli"

#ifndef CS_KERNEL_SIZE_X
#define CS_KERNEL_SIZE_X 8
#endif

#ifndef CS_KERNEL_SIZE_Y
#define CS_KERNEL_SIZE_Y 8
#endif

#ifndef CS_KERNEL_SIZE_Z
#define CS_KERNEL_SIZE_Z 1
#endif

#define SSAO_SAMPLE_COUNT 14

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

	uint2 OutputSize, NoiseSize;
	NoiseMap.GetDimensions(NoiseSize.x, NoiseSize.y);
	c_OcclusionOutput.GetDimensions(OutputSize.x, OutputSize.y);
	
	// SSAO, for deferred rendering using depth, normal and noise map inside compute shader
	float2 Uv = float2(DTID.xy) / OutputSize;
	float2 NoiseUv = float2(DTID.xy) / NoiseSize;
	float2 NoiseScale = float2(OutputSize) / float2(NoiseSize);
	
	float3 Normal = NormalMap.SampleLevel(s_Sampler_LinearClamp, Uv, 0.f).xyz;
	
	float Depth = DepthMap.SampleLevel(s_Sampler_LinearBorder, Uv, 0.f).x;
	float3 Position = float3(Depth * Uv.x, Depth * Uv.y, Depth);
	//float3 Position = UVToViewPosition(Uv, Depth, g_FrameData.ProjectionInverse);
	
	float3 RandomVec = normalize(NoiseMap.SampleLevel(s_Sampler_LinearWrap, NoiseUv * NoiseScale, 0.f).xyz * 2.f - 1.f);
	
	float3 Tangent = normalize(RandomVec - Normal * dot(RandomVec, Normal));
	float3 Bitangent = cross(Normal, Tangent);
	
	float3x3 TBN = float3x3(Tangent, Bitangent, Normal);
	
	float Occlusion = 0.0f;
	
	for (int i = 0; i < SSAO_SAMPLE_COUNT; ++i)
	{
		float3 Sample = mul(TBN, c_SSAOParams.Samples[i].xyz);
		Sample = Position + Sample * c_SSAOParams.Radius;
		
		float4 Offset = float4(Sample, 1.0f);
		Offset = mul(g_FrameData.Projection, Offset);
		Offset.xy /= Offset.w;
		Offset.xy = (Offset.xy * float2(1.0f, -1.0f)) * 0.5f + 0.5f;
		
		float SampleDepth = DepthMap.SampleLevel(s_Sampler_LinearBorder, Offset.xy, 0).x;
		
		float RangeCheck = abs(Position.z - SampleDepth) < c_SSAOParams.Radius ? 1.0f : 0.0f;
		Occlusion += (SampleDepth <= Sample.z ? 1.0f : 0.0f) * RangeCheck;
	}
	
	Occlusion = saturate(1.0f - Occlusion / SSAO_SAMPLE_COUNT);
	Occlusion = pow(Occlusion, c_SSAOParams.Contrast);
	
	Occlusion = lerp(c_SSAOParams.Bias, 1.0f, Occlusion);
	Occlusion *= c_SSAOParams.Magnitude;
	
	c_OcclusionOutput[DTID.xy] = Occlusion;
}