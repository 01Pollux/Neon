
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

// --------------------
// Structures
// --------------------

struct PerFrameData
{
	matrix World;
	
	matrix View;
	matrix Projection;
	matrix ViewProjection;
	
	matrix ViewInverse;
	matrix ProjectionInverse;
	matrix ViewProjectionInverse;

	float2 ScreenResolution;
	
	float EngineTime;
	float GameTime;
	float DeltaTime;
};

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

ConstantBuffer<PerFrameData> c_PerFrameData : register(b0, space0);
ConstantBuffer<SSAOParams> c_SSAOParams : register(b1, space0);

Texture2D c_NormalMap : register(t0, space0);
Texture2D<float> c_DepthMap : register(t1, space0);
Texture2D<float3> c_NoiseMap : register(t2, space0);

RWTexture2D<float> c_OcclusionOutput : register(u0, space0);

//

SamplerState c_Sampler_PointWrap : register(s0, space0);
SamplerState c_Sampler_PointClamp : register(s1, space0);
SamplerState c_Sampler_LinearWrap : register(s2, space0);
SamplerState c_Sampler_LinearClamp : register(s3, space0);
SamplerState c_Sampler_AnisotropicWrap : register(s4, space0);
SamplerState c_Sampler_AnisotropicClamp : register(s5, space0);

//

float3 UVToViewPosition(
	float2 UV,
	float Depth)
{
	float2 ScreenUV = UV * 2.0f - 1.0f;
	ScreenUV.y *= -1.0f;
	
	float4 ScreenPos = float4(ScreenUV, Depth, 1.0f);
	float4 WorldPos = mul(ScreenPos, c_PerFrameData.ProjectionInverse);
	
	return WorldPos.xyz / WorldPos.w;
}

float3 UVToWorldPosition(
	float2 UV,
	float Depth)
{
	float2 ScreenUV = UV * 2.0f - 1.0f;
	ScreenUV.y *= -1.0f;
	
	float4 ScreenPos = float4(ScreenUV, Depth, 1.0f);
	float4 WorldPos = mul(ScreenPos, c_PerFrameData.ViewProjectionInverse);
	
	return WorldPos.xyz / WorldPos.w;
}

//

[numthreads(CS_KERNEL_SIZE_X, CS_KERNEL_SIZE_Y, CS_KERNEL_SIZE_Z)]
void CS_Main(
	uint3 DTID : SV_DispatchThreadID)
{
	float2 Resolution = c_PerFrameData.ScreenResolution * c_SSAOParams.ResolutionFactor;
	float2 UV = DTID.xy / Resolution;
	
	float3 Normal = c_NormalMap.Sample(c_Sampler_LinearClamp, UV).xyz;
	Normal = Normal * 2.f - 1.f;
	Normal = normalize(Normal);
	
	float Depth = c_DepthMap.Sample(c_Sampler_LinearClamp, UV).x;
	float3 Position = UVToViewPosition(UV, Depth);
	
	float3 Random = normalize(c_NoiseMap.Sample(c_Sampler_LinearWrap, UV).xyz * 2.f - 1.f);
	
	float3 Tangent = normalize(Random - Normal * dot(Random, Normal));
	float3 Bitangent = cross(Normal, Tangent);
	float3x3 TBN = float3x3(Tangent, Bitangent, Normal);

	float Occlusion = (float) SSAO_SAMPLE_COUNT;

	[unroll(SSAO_SAMPLE_COUNT)]
	for (int i = 0; i < SSAO_SAMPLE_COUNT; ++i)
	{
		float3 SamplePos = mul(c_SSAOParams.Samples[i].xyz, TBN);
		SamplePos = Position + SamplePos * c_SSAOParams.Radius;
		
		float3 SampleUV = mul(float4(SamplePos, 1.f), c_PerFrameData.ViewProjection).xyw;
		SampleUV.xy = ((SampleUV.xy / SampleUV.z) * .5f * float2(1.f, -1.f) + .5f);
		
		float SampleDepth = c_DepthMap.Sample(c_Sampler_LinearClamp, SampleUV.xy);
		SampleDepth = UVToViewPosition(SampleUV.xy, SampleDepth).z;
		
		float Occluded = step(SampleDepth + c_SSAOParams.Bias, SamplePos.z);
		float Intensity = smoothstep(.0f, 1.f, c_SSAOParams.Radius * (Position.z - SampleDepth));
		
		Occlusion -= Occluded * Intensity;
	}
	
	Occlusion /= SSAO_SAMPLE_COUNT;
	Occlusion = pow(Occlusion, c_SSAOParams.Magnitude);
	Occlusion = c_SSAOParams.Contrast * (Occlusion - .5f) + .5f;
	
	c_OcclusionOutput[DTID.xy] = Occlusion;
}