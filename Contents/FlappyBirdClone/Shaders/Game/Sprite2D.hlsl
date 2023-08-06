
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
};

struct PerObjectData
{
	matrix World;
	float4 Color;
	int TextureIndex;
	int _Pad;
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
	Ps.Position = float4(Vs.Position, 0.f, 1.f);
	Ps.Position = mul(Ps.Position, g_SpriteData[Vs.SpriteIndex].World);
	Ps.Position = mul(Ps.Position, g_FrameData.ViewProjection);
	Ps.Position.z = 0.f;
	Ps.TexCoord = Vs.TexCoord;
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

float4 PS_Main(PSInput Ps) : SV_TARGET
{
	int TextureIndex = g_SpriteData[Ps.SpriteIndex].TextureIndex;
	float4 Color = g_SpriteData[Ps.SpriteIndex].Color;
	Color *= p_SpriteTextures[TextureIndex].Sample(p_Sampler_PointWrap, Ps.TexCoord);
	clip(Color.a - 0.1f);
	return Color;
}
