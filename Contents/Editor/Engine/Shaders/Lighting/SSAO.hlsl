
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
	                              
	float2 DispatchIndex = (float2(DTID.xy) + 0.5f);
	float2 Uv = DispatchIndex / OutputSize;
	float2 NoiseUv = DispatchIndex / NoiseSize;
	float2 NoiseScale = float2(OutputSize) / float2(NoiseSize);
	
	float3 Normal = normalize(NormalMap.SampleLevel(s_Sampler_LinearClamp, Uv, 0.f).xyz);
	float3 Random = normalize(NoiseMap.SampleLevel(s_Sampler_PointWrap, NoiseUv * NoiseScale, 0.f).xyz * 2.f - 1.f);
	
	float Depth = DepthMap.SampleLevel(s_Sampler_LinearClamp, Uv, 0.f).x;
	float3 Position = UVToViewPosition(Uv, Depth, g_FrameData.ProjectionInverse);
	                                                            
	float3 Tangent = normalize(Random - Normal * dot(Random, Normal));
	float3 Bitangent = cross(Normal, Tangent);
	float3x3 TBN = float3x3(Tangent, Bitangent, Normal);

	float Occlusion = (float) 0.f;

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
		
		float RangeCheck = smoothstep(0.0, 1.0, c_SSAOParams.Radius / abs(Position.z - SampleDepth));
		Occlusion += RangeCheck * step(SampleDepth, SamplePos.z - 0.01);
	}
	                                                                                     
	Occlusion = pow(abs(1.f - Occlusion / SSAO_SAMPLE_COUNT), c_SSAOParams.Magnitude);
	
	c_OcclusionOutput[DTID.xy] = Occlusion;
}