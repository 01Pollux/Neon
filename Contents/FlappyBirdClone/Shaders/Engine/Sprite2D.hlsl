
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
	matrix TextureTransform;
	float4 Color;
	int TextureIndex;
	int3 _Pad;
};

//

struct VSInput
{
	float2 Position : POSITION;
	float2 TexCoord : TEX_COORD;
	int SpriteIndex : SPRITE_INDEX;
};

struct PSInput
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEX_COORD;
	nointerpolation int SpriteIndex : SPRITE_INDEX;
};

struct PSOutput
{
	float4 Albedo : SV_TARGET0;
	float4 Normal : SV_TARGET1;
	float4 Emissive : SV_TARGET2;
};

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
StructuredBuffer<PerObjectData> g_SpriteData : register(t0, space1);

// --------------------
// Vertex Shader
// --------------------

PSInput VS_Main(VSInput Vs)
{
	PSInput Ps;
	Ps.Position = mul(
		mul(float4(Vs.Position, 0.f, 1.f),
			g_SpriteData[Vs.SpriteIndex].World),
		g_FrameData.ViewProjection);
	Ps.TexCoord = mul(float4(Vs.TexCoord, 0.f, 1.f), g_SpriteData[Vs.SpriteIndex].TextureTransform).xy;
	Ps.SpriteIndex = Vs.SpriteIndex;
	return Ps;
}

// --------------------
// Pixel Shader
// --------------------

Texture2D p_SpriteTextures[] : register(t0, space2);

SamplerState p_Sampler_PointWrap : register(s0, space0);
SamplerState p_Sampler_PointClamp : register(s1, space0);
SamplerState p_Sampler_LinearWrap : register(s2, space0);
SamplerState p_Sampler_LinearClamp : register(s3, space0);
SamplerState p_Sampler_AnisotropicWrap : register(s4, space0);
SamplerState p_Sampler_AnisotropicClamp : register(s5, space0);

[earlydepthstencil]
PSOutput PS_Main(PSInput Ps)
{
	int TextureIndex = g_SpriteData[Ps.SpriteIndex].TextureIndex;
	float4 Color = g_SpriteData[Ps.SpriteIndex].Color;
	Color *= p_SpriteTextures[TextureIndex].Sample(p_Sampler_PointWrap, Ps.TexCoord);
	clip(Color.a - 0.1f);
	
	return GBufferPack(
		Color,
		Color,
		Color);
}
