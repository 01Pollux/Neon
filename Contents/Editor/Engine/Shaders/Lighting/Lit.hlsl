
#include "../Common/Frame.hlsli"
#include "../Common/Utils.hlsli"
#include "../Common/Light.hlsli"
#include "../Common/StdMaterial.hlsli"

enum
{
	TEXTURE_OFFSET_ALBEDO,
	TEXTURE_OFFSET_NORMAL,
	TEXTURE_OFFSET_SPECULAR,
	TEXTURE_OFFSET_EMISSIVE
};

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
	float4 PositionWS : POSITION_WORLD;
	
	float3 NormalWS : NORMAL_WORLD;
	
	float3 TangentWS : TANGENT_WORLD;
	float3 BitangentWS : BITANGENT_WORLD;
	
	float2 TexCoord : TEX_COORD;
	nointerpolation uint InstanceId : INSTANCE_INDEX;
};


//

// --------------------
// Vertex Shader
// --------------------

StructuredBuffer<PerObjectData> v_ObjectData : register(t0, space2);


PSInput VS_Main(VSInput Vs)
{
	PerObjectData Object = v_ObjectData[Vs.InstanceId];
	PSInput Ps = (PSInput) 0;
	
	Ps.PositionWS = mul(float4(Vs.Position, 1.f), Object.World);
	Ps.Position = mul(Ps.PositionWS, g_FrameData.ViewProjection);
	
	Ps.NormalWS = mul(float4(Vs.Normal, 0.f), Object.World).xyz;
	
	Ps.TangentWS = mul(float4(Vs.Tangent, 0.f), Object.World).xyz;
	Ps.BitangentWS = mul(float4(Vs.Bitangent, 0.f), Object.World).xyz;
	Ps.TexCoord = Vs.TexCoord;
	Ps.InstanceId = Vs.InstanceId;
	
	return Ps;
}


// --------------------
// Pixel Shader
// --------------------

StructuredBuffer<Light> p_Lights : register(t0, space1);
StructuredBuffer<uint> p_LightIndexList : register(t1, space1);
Texture2D<uint2> p_LightGrid : register(t2, space1);

StructuredBuffer<PerMaterialData> p_MaterialData : register(t0, space2);
Texture2D p_TextureTable[] : register(t0, space3);


[earlydepthstencil]
float4 PS_Main(PSInput Ps, bool IsFrontFace : SV_IsFrontFace) : SV_Target
{
	PerMaterialData Material = p_MaterialData[Ps.InstanceId];
	
	float3 Albedo = Material.Albedo;
	[branch]
	if (Material.Flags & MATERIAL_FLAG_ALBEDO_MAP)
	{
		Texture2D AlbedoMap = p_TextureTable[Material.AlbedoMapIndex + TEXTURE_OFFSET_ALBEDO];
		float4 Color = AlbedoMap.Sample(s_Sampler_LinearWrap, Ps.TexCoord);
		if (Color.a < 0.1f)
		{
			discard;
		}
		Albedo.rgb *= Color.rgb;
	}
	
	float4 Normal = float4(Ps.NormalWS, 0.f);
	
	[branch]
	if (Material.Flags & MATERIAL_FLAG_NORMAL_MAP)
	{
		Texture2D NormalMap = p_TextureTable[Material.AlbedoMapIndex + TEXTURE_OFFSET_NORMAL];
		float3 BumpNormal = NormalMap.Sample(s_Sampler_LinearWrap, Ps.TexCoord).xyz * 2.f - 1.f;
		float3x3 TBN = float3x3(Ps.TangentWS, Ps.BitangentWS, BumpNormal);
		Normal.xyz = mul(Normal.xyz, TBN);
	}
	
	Normal = normalize(Normal);
	
	float4 Specular = float4(Material.Specular, 1.f);
	[branch]
	if (Material.Flags & MATERIAL_FLAG_SPECULAR_MAP)
	{
		Texture2D SpecularMap = p_TextureTable[Material.AlbedoMapIndex + TEXTURE_OFFSET_SPECULAR];
		Specular *= SpecularMap.Sample(s_Sampler_LinearWrap, Ps.TexCoord);
	}
	
	float4 Emissive = float4(Material.Emissive, 1.f);
	[branch]
	if (Material.Flags & MATERIAL_FLAG_EMISSIVE_MAP)
	{
		Texture2D EmissiveMap = p_TextureTable[Material.AlbedoMapIndex + TEXTURE_OFFSET_EMISSIVE];
		Emissive = EmissiveMap.Sample(s_Sampler_LinearWrap, Ps.TexCoord);
	}

	//
	
	float4 TargetToEye = normalize(float4(g_FrameData.World._41_42_43, 1.f) - Ps.PositionWS);

	// Perform lighting
	
	uint2 LightInfo = p_LightGrid[uint2(floor(Ps.Position.xy / LIGHT_CLUSTER_SIZE))];
	uint StartOffset = LightInfo.x;
	uint LightCount = LightInfo.y;
	
	LightingResult LightRes = (LightingResult) 0;
	for (uint i = 0; i < LightCount; i++)
	{
		Light CurLight = p_Lights[p_LightIndexList[StartOffset + i]];
		CurLight.Process(TargetToEye, Ps.PositionWS, Normal, LightRes);
	}
	
	return float4(Albedo, 1.f) * LightRes.Diffuse + Emissive + Specular * LightRes.Specular;
}
