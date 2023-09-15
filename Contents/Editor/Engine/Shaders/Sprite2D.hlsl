#include "Common/Frame.hlsli"
#include "Common/Utils.hlsli"
#include "Common/GBuffer.hlsli"

// --------------------
// Structures
// --------------------

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
	float2 PositionInScene : POSITION;
	float2 TexCoord : TEX_COORD;
	nointerpolation int SpriteIndex : SPRITE_INDEX;
};

// --------------------
// Global
// --------------------

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
	Ps.PositionInScene = Ps.Position.xy;
	Ps.TexCoord = mul(float4(Vs.TexCoord, 0.f, 1.f), g_SpriteData[Vs.SpriteIndex].TextureTransform).xy;
	Ps.SpriteIndex = Vs.SpriteIndex;
	return Ps;
}

// --------------------
// Pixel Shader
// --------------------

Texture2D p_SpriteTextures[] : register(t0, space2);

[earlydepthstencil]
PSOutput PS_Main(PSInput Ps)
{
	int TextureIndex = g_SpriteData[Ps.SpriteIndex].TextureIndex;
	float4 Color = g_SpriteData[Ps.SpriteIndex].Color;
	Color *= p_SpriteTextures[TextureIndex].Sample(s_Sampler_PointWrap, Ps.TexCoord);
	clip(Color.a - 0.1f);
	
	return GBufferPack(
		Color.rgb,
		1.f,
		(float3) 0,
		1.f,
		(float4) 0);
}
