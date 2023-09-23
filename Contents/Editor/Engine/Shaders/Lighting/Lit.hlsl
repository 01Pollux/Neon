
#include "../Common/Standard.hlsli"
#include "../Common/Frame.hlsli"
#include "../Common/Utils.hlsli"
#include "../Common/StdMaterial.hlsli"
#include "../Common/GBuffer.hlsli"

//

struct PerObjectData
{
	matrix World;
};

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
};

struct CBInfo
{
	uint PerObjectOffset;
};

ConstantBuffer<CBInfo> g_CBInfo : register(b0, space1);

//

// --------------------
// Vertex Shader
// --------------------

PSInput VS_Main(VSInput Vs)
{
	PerObjectData Object = DescriptorTable::LoadObject_R<PerObjectData>(g_CBInfo.PerObjectOffset);
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

[earlydepthstencil]
PSOutput PS_Main(PSInput Ps, bool IsFrontFace : SV_IsFrontFace)
{
	PerMaterialData Material = DescriptorTable::LoadObject_R<PerMaterialData>(g_CBInfo.PerObjectOffset + 1);
	
	float3 Albedo = Material.Albedo;
	[branch]
	if (Material.Flags & MATERIAL_FLAG_ALBEDO_MAP)
	{
		Texture2D AlbedoMap = DescriptorTable::Load2D(Material.AlbedoMapIndex);
		
		float4 Color = AlbedoMap.Sample(s_Sampler_LinearWrap, Ps.TexCoord);
		if (Color.a < 0.1f)
		{
			discard;
		}
		Albedo.rgb *= Color.rgb;
	}
	
	float3 Normal = Ps.NormalWS;
	
	[branch]
	if (Material.Flags & MATERIAL_FLAG_NORMAL_MAP)
	{
		Texture2D NormalMap = DescriptorTable::Load2D(Material.NormalMapIndex);
		
		float3 BumpNormal = NormalMap.Sample(s_Sampler_LinearWrap, Ps.TexCoord).xyz * 2.f - 1.f;
		float3x3 TBN = float3x3(Ps.TangentWS, Ps.BitangentWS, BumpNormal);
		Normal = mul(Normal, TBN);
	}
	
	Normal = normalize(mul(Normal, (float3x3) g_FrameData.View));
	
	float4 Specular = float4(Material.Specular, 1.f);
	[branch]
	if (Material.Flags & MATERIAL_FLAG_SPECULAR_MAP)
	{
		Texture2D SpecularMap = DescriptorTable::Load2D(Material.SpecularMapIndex);
		Specular *= SpecularMap.Sample(s_Sampler_LinearWrap, Ps.TexCoord);
	}
	
	float4 Emissive = float4(Material.Emissive, 1.f);
	[branch]
	if (Material.Flags & MATERIAL_FLAG_EMISSIVE_MAP)
	{
		Texture2D EmissiveMap = DescriptorTable::Load2D(Material.EmissiveMapIndex);
		Emissive = EmissiveMap.Sample(s_Sampler_LinearWrap, Ps.TexCoord);
	}
	
	return GBufferPack(
		Albedo,
		0.75f, // TODO
		Normal,
		0.75f, // TODO
		Emissive);
}
