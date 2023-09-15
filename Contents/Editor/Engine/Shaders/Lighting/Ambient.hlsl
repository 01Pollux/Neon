
#include "../Common/Frame.hlsli"
#include "../Common/Utils.hlsli"
#include "../Common/Packing.hlsli"

#ifndef CS_KERNEL_SIZE_X
#define CS_KERNEL_SIZE_X 16
#endif

#ifndef CS_KERNEL_SIZE_Y
#define CS_KERNEL_SIZE_Y 16
#endif

#ifndef CS_KERNEL_SIZE_Z
#define CS_KERNEL_SIZE_Z 1
#endif


//

struct AmbientParams
{
	uint Color;
	
	uint DiffuseRoughnessMap;
	uint EmissiveFactorMap;
	uint DepthMap;
	uint AmbientOcclusionMap;
};

ConstantBuffer<AmbientParams> c_AmbientParams : register(b0, space1);
Texture2D c_TexturesMap[] : register(t0, space1);
RWTexture2D<float4> c_OutputTexture : register(u0, space1);

// --------------------
// Compute Shader
// --------------------

[numthreads(CS_KERNEL_SIZE_X, CS_KERNEL_SIZE_Y, CS_KERNEL_SIZE_Z)]
void CS_Main(
	uint3 DTID : SV_DispatchThreadID)
{
	Texture2D DiffuseRoughnessMap = c_TexturesMap[0];
	Texture2D EmissiveFactorMap = c_TexturesMap[1];
	Texture2D DepthMap = c_TexturesMap[2];
	Texture2D AmbientMap = c_TexturesMap[3];
	
	int2 Size;
	c_OutputTexture.GetDimensions(Size.x, Size.y);
	float2 UV = ((float2) DTID.xy + 0.5f) / Size;
	
	// Outside of the screen
	if (all(UV) < 0.f || all(UV) > 1.f)
	{
		return;
	}
	
	float4 Albedo = float4(DiffuseRoughnessMap.SampleLevel(s_Sampler_LinearWrap, UV, 0).rgb, 1.f);
	float4 EmissiveData = EmissiveFactorMap.SampleLevel(s_Sampler_LinearWrap, UV, 0);
	float4 Emissive = float4(EmissiveData.rgb * EmissiveData.a * 255, 1.f);
	float AmbientOcclusion = AmbientMap.SampleLevel(s_Sampler_LinearWrap, UV, 0).r;
	float4 Color = UnpackUintColor(c_AmbientParams.Color) * AmbientOcclusion;

	c_OutputTexture[DTID.xy] = Color * Albedo + Emissive;
}