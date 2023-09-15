
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
	
	uint InstanceId : INSTANCE_INDEX;
};


//

// --------------------
// Vertex Shader
// --------------------

StructuredBuffer<PerObjectData> v_ObjectData : register(t0, space1);


PSInput VS_Main(VSInput Vs)
{
	PerObjectData Object = v_ObjectData[Vs.InstanceId];
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

StructuredBuffer<PerMaterialData> p_MaterialData : register(t0, space1);

Texture2D<float4> p_AlbedoMap[] : register(t0, space2);
Texture2D<float4> p_NormalMap[] : register(t0, space3);
Texture2D<float4> p_SpecularMap[] : register(t0, space4);
Texture2D<float4> p_EmissiveMap[] : register(t0, space5);


[earlydepthstencil]
PSOutput PS_Main(PSInput Ps, bool IsFrontFace : SV_IsFrontFace)
{
	PerMaterialData Material = p_MaterialData[Ps.InstanceId];
	
	float3 Albedo = Material.Albedo;
	[branch]
	if (Material.Flags & MATERIAL_FLAG_ALBEDO_MAP)
	{
		Texture2D AlbedoMap = p_AlbedoMap[Material.AlbedoMapIndex];
		float4 Color = AlbedoMap.Sample(s_Sampler_LinearWrap, Ps.TexCoord);
		if (Color.a < 0.1f)
		{
			discard;
		}
		Albedo.rgb *= Color.rgb;
	}
	
	float3 Normal;
	[branch]
	if (Material.Flags & MATERIAL_FLAG_NORMAL_MAP)
	{
		Texture2D NormalMap = p_NormalMap[Material.NormalMapIndex];
		Normal = NormalMap.Sample(s_Sampler_LinearWrap, Ps.TexCoord).xyz;
		Normal = Normal * 2.f - 1.f;
		Normal = normalize(Normal);
	}
	else
	{
		Normal = normalize(Ps.NormalWS);
	}
	// Invert normal if front face
	if (IsFrontFace)
	{
		Normal.z *= -1.f;
	}
	
	float4 Specular = float4(Material.Specular, 1.f);
	[branch]
	if (Material.Flags & MATERIAL_FLAG_SPECULAR_MAP)
	{
		Texture2D SpecularMap = p_SpecularMap[Material.SpecularMapIndex];
		Specular *= SpecularMap.Sample(s_Sampler_LinearWrap, Ps.TexCoord);
	}
	
	float4 Emissive = float4(Material.Emissive, 1.f);
	[branch]
	if (Material.Flags & MATERIAL_FLAG_EMISSIVE_MAP)
	{
		Texture2D EmissiveMap = p_EmissiveMap[Material.EmissiveMapIndex];
		Emissive = EmissiveMap.Sample(s_Sampler_LinearWrap, Ps.TexCoord);
	}
	
	return GBufferPack(
		Albedo,
		0.75f, // TODO
		Normal,
		0.75f, // TODO
		Emissive);
}
