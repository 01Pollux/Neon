#include "../Common/StdMaterial.hlsli"

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
	float4 PositionWS : POSITION_WORLD;
	
	float3 NormalWS : NORMAL_WORLD;
	
	float3 TangentWS : TANGENT_WORLD;
	float3 BitangentWS : BITANGENT_WORLD;
	
	float2 TexCoord : TEX_COORD;
	nointerpolation uint InstanceId : INSTANCE_INDEX;
};

//

struct MaterialData : StdMaterialData
{
};
NEON_MATERIAL_LOCAL(MaterialData);

//

// --------------------
// Vertex Shader
// --------------------



PSInput VS_Main(VSInput Vs)
{
	PerInstanceData Object = g_InstanceData[Vs.InstanceId];
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

[earlydepthstencil]
float4 PS_Main(PSInput Ps) : SV_Target
{
	MaterialData Material = g_Local[Ps.InstanceId];
	
	float3 Albedo = Material.Color_Albedo;
	[branch]
	if (Material._Tex2D_AlbedoIndex != -1)
	{
		Texture2D AlbedoMap = g_Texture2D[Material._Tex2D_AlbedoIndex];
		float4 Color = AlbedoMap.Sample(s_Sampler_LinearWrap, Ps.TexCoord);
		if (Color.a < 0.1f)
		{
			discard;
		}
		Albedo.rgb *= Color.rgb;
	}
	
	float4 Normal = float4(Ps.NormalWS, 0.f);
	
	[branch]
	if (Material._Tex2D_NormalIndex != -1)
	{
		Texture2D NormalMap = g_Texture2D[Material._Tex2D_NormalIndex];
		float3 BumpNormal = NormalMap.Sample(s_Sampler_LinearWrap, Ps.TexCoord).xyz * 2.f - 1.f;
		float3x3 TBN = float3x3(Ps.TangentWS, Ps.BitangentWS, BumpNormal);
		Normal.xyz = mul(Normal.xyz, TBN);
	}
	
	Normal = normalize(Normal);
	
	float4 Specular = float4(Material.Color_Specular, 1.f);
	[branch]
	if (Material._Tex2D_SpecularIndex != -1)
	{
		Texture2D SpecularMap = g_Texture2D[Material._Tex2D_SpecularIndex];
		Specular *= SpecularMap.Sample(s_Sampler_LinearWrap, Ps.TexCoord);
	}
	
	float4 Emissive = float4(Material.Color_Emissive, 1.f);
	[branch]
	if (Material._Tex2D_EmissiveIndex != -1)
	{
		Texture2D EmissiveMap = g_Texture2D[Material._Tex2D_EmissiveIndex];
		Emissive = EmissiveMap.Sample(s_Sampler_LinearWrap, Ps.TexCoord);
	}

	//
	
	float4 TargetToEye = normalize(float4(g_FrameData.World._41_42_43, 1.f) - Ps.PositionWS);

	// Perform lighting
	
	uint2 LightInfo = _LightGrid[uint2(floor(Ps.Position.xy / LIGHT_CLUSTER_SIZE))];
	uint StartOffset = LightInfo.x;
	uint LightCount = LightInfo.y;
	
	LightingResult LightRes = (LightingResult) 0;
	for (uint i = 0; i < LightCount; i++)
	{
		Light CurLight = _Lights[_LightIndexList[StartOffset + i]];
		CurLight.Process(TargetToEye, Ps.PositionWS, Normal, LightRes);
	}
	
	return float4(Albedo, 1.f) * LightRes.Diffuse + Emissive + Specular * LightRes.Specular;
}
