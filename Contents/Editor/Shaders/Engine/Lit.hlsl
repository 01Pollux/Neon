
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
	
	float EngineTime;
	float GameTime;
	float DeltaTime;
};

struct PerObjectData
{
	matrix World;
};

//

struct VSInput
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Bitangent : BITANGENT;
	float2 TexCoord : TEXCOORD;
};

struct PSInput
{
	float4 Position : SV_POSITION;
	float2 PositionInScene : POSITION;
	float2 TexCoord : TEX_COORD;
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

ConstantBuffer<PerObjectData> v_PerObjectData : register(b1, space1);

PSInput VS_Main(VSInput Vs)
{
	PSInput Ps;
	Ps.Position = mul(
		mul(
			float4(Vs.Position, 1.f),
			v_PerObjectData.World),
		g_FrameData.ViewProjection);
	Ps.PositionInScene = Ps.Position.xy;
	Ps.TexCoord = Vs.TexCoord;
	return Ps;
}

// --------------------
// Pixel Shader
// --------------------

Texture2D<float4> p_AlbedoMap : register(t0, space2);
Texture2D<float4> p_NormalMap : register(t0, space3);
Texture2D<float4> p_SpecularMap : register(t0, space4);
Texture2D<float4> p_EmissiveMap : register(t0, space5);

SamplerState p_Sampler_PointWrap : register(s0, space0);
SamplerState p_Sampler_PointClamp : register(s1, space0);
SamplerState p_Sampler_LinearWrap : register(s2, space0);
SamplerState p_Sampler_LinearClamp : register(s3, space0);
SamplerState p_Sampler_AnisotropicWrap : register(s4, space0);
SamplerState p_Sampler_AnisotropicClamp : register(s5, space0);

[earlydepthstencil]
PSOutput PS_Main(PSInput Ps)
{
	Texture2D AlbedoMap = p_AlbedoMap;
	Texture2D NormalMap = p_NormalMap;
	Texture2D SpecularMap = p_SpecularMap;
	Texture2D EmissiveMap = p_EmissiveMap;

	float4 Albedo = AlbedoMap.Sample(p_Sampler_LinearWrap, Ps.TexCoord);
	clip(Albedo.a - 0.1f);

	float4 Normal = NormalMap.Sample(p_Sampler_LinearWrap, Ps.TexCoord);
	Normal = float4(Normal.xyz * 2.f - 1.f, 0.f);
	Normal = normalize(Normal);
	
	float4 Specular = SpecularMap.Sample(p_Sampler_LinearWrap, Ps.TexCoord);
	float4 Emissive = EmissiveMap.Sample(p_Sampler_LinearWrap, Ps.TexCoord);
	
	return GBufferPack(
		Albedo,
		Normal,
		Emissive);
}
