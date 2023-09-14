
#include "../Common/Frame.hlsli"
#include "../Common/StdMaterial.hlsli"

struct PerObjectData
{
	matrix World;
};

//

struct VSInput
{
	uint InstanceId : SV_InstanceID;
	
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Bitangent : BITANGENT;
	float2 TexCoord : TEXCOORD;
};

struct PSInput
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEX_COORD;
	
	float3 NormalLS : NORMAL_LOCAL;
	float3 NormalWS : NORMAL_WORLD;
	
	float3 TangentWS : TANGENT_WORLD;
	float3 BitangentWS : BITANGENT_WORLD;
	
	uint InstanceId : INSTANCE_INDEX;
};

struct PSOutput
{
	float4 Albedo : SV_TARGET0;
	float4 Normal : SV_TARGET1;
	float4 Emissive : SV_TARGET2;
};


//

PSOutput GBufferPack(
	float4 Albedo,
	float4 Normal,
	float4 Emissive)
{
	PSOutput Out;
	Out.Albedo = Albedo;
	Out.Normal = Normal;
	Out.Emissive = Emissive;
	return Out;
}

// --------------------
// Global
// --------------------

ConstantBuffer<PerFrameData> g_FrameData : register(b0, space1);


// --------------------
// Vertex Shader
// --------------------

ByteAddressBuffer v_ObjectData : register(t0, space1);

PerObjectData LoadPerObjectData(uint InstanceId)
{
	uint Offset = InstanceId * sizeof(PerObjectData);
	return v_ObjectData.Load < PerObjectData > (Offset);
}


PSInput VS_Main(VSInput Vs)
{
	PerObjectData Object = LoadPerObjectData(Vs.InstanceId);
	PSInput Ps = (PSInput) 0;
	
	Ps.Position = mul(
		mul(
			float4(Vs.Position, 1.f),
			Object.World),
		g_FrameData.ViewProjection);
	Ps.TexCoord = Vs.TexCoord;
	
	Ps.NormalLS = Vs.Normal;
	Ps.NormalWS = mul(float4(Vs.Normal, 0.f), (float4x3) Object.World);
	
	Ps.TangentWS = mul(float4(Vs.Tangent, 0.f), (float4x3) Object.World);
	Ps.BitangentWS = mul(float4(Vs.Bitangent, 0.f), (float4x3) Object.World);
	
	return Ps;
}


// --------------------
// Pixel Shader
// --------------------

ByteAddressBuffer p_MaterialData : register(t0, space1);

PerMaterialData LoadPerMaterialData(uint InstanceId)
{
	uint Offset = InstanceId * sizeof(PerMaterialData);
	return p_MaterialData.Load < PerMaterialData > (Offset);
}

Texture2D<float4> p_AlbedoMap[] : register(t0, space2);
Texture2D<float4> p_NormalMap[] : register(t0, space3);
Texture2D<float4> p_SpecularMap[] : register(t0, space4);
Texture2D<float4> p_EmissiveMap[] : register(t0, space5);

SamplerState p_Sampler_PointWrap : register(s0, space0);
SamplerState p_Sampler_PointClamp : register(s1, space0);
SamplerState p_Sampler_LinearWrap : register(s2, space0);
SamplerState p_Sampler_LinearClamp : register(s3, space0);
SamplerState p_Sampler_AnisotropicWrap : register(s4, space0);
SamplerState p_Sampler_AnisotropicClamp : register(s5, space0);


[earlydepthstencil]
PSOutput PS_Main(PSInput Ps, bool IsFrontFace : SV_IsFrontFace)
{
	PerMaterialData Material = LoadPerMaterialData(Ps.InstanceId);
	
	float4 Albedo = float4(Material.Albedo, 1.f);
	[branch]
	if (Material.Flags & MATERIAL_FLAG_ALBEDO_MAP)
	{
		Texture2D AlbedoMap = p_AlbedoMap[Material.AlbedoMapIndex];
		Albedo *= AlbedoMap.Sample(p_Sampler_LinearWrap, Ps.TexCoord);
	}
	
	if (Albedo.a < 0.1f)
	{
		discard;
	}
	
	float4 Normal;
	[branch]
	if (Material.Flags & MATERIAL_FLAG_NORMAL_MAP)
	{
		Texture2D NormalMap = p_NormalMap[Material.NormalMapIndex];
		Normal = NormalMap.Sample(p_Sampler_LinearWrap, Ps.TexCoord);
		Normal = float4(Normal.xyz * 2.f - 1.f, 0.f);
		Normal = normalize(Normal);
	}
	else
	{
		Normal = float4(normalize(Ps.NormalWS), 0.f);
	}
	
	float4 Specular = float4(Material.Specular, 1.f);
	[branch]
	if (Material.Flags & MATERIAL_FLAG_SPECULAR_MAP)
	{
		Texture2D SpecularMap = p_SpecularMap[Material.SpecularMapIndex];
		Specular *= SpecularMap.Sample(p_Sampler_LinearWrap, Ps.TexCoord);
	}
	
	float4 Emissive = float4(Material.Emissive, 1.f);
	[branch]
	if (Material.Flags & MATERIAL_FLAG_EMISSIVE_MAP)
	{
		Texture2D EmissiveMap = p_EmissiveMap[Material.EmissiveMapIndex];
		Emissive = EmissiveMap.Sample(p_Sampler_LinearWrap, Ps.TexCoord);
	}
	
	return GBufferPack(
		Albedo,
		Normal,
		Emissive);
}
