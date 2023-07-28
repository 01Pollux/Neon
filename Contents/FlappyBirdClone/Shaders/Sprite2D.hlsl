
struct PerFrameData
{
	matrix View;
	matrix Projection;
	matrix ViewProjection;
	matrix ViewInverse;
	matrix ProjectionInverse;
	matrix ViewProjectionInverse;
	
	float3 CameraPosition;
	float3 CameraDirection;
	float3 CameraUp;
};

struct PerObjectData
{
	matrix World;
	float4 Color;
};

//

struct VSInput
{
	float3 Position : POSITION;
	float2 TexCoord : TEX_COORD;
	int SpriteIndex : SPRITE_INDEX;
};

struct VSOutput
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

VSOutput VS_Main(VSInput Input)
{
	VSOutput Output;
	Output.Position = float4(Input.Position, 1.0f);
	Output.Position = mul(Output.Position, g_SpriteData[Input.SpriteIndex].World);
	Output.Position = mul(Output.Position, g_FrameData.ViewProjection);
	Output.TexCoord = Input.TexCoord;
	Output.SpriteIndex = Input.SpriteIndex;
	return Output;
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

float4 PS_Main(VSOutput Input) : SV_TARGET
{
	float4 Color = p_SpriteTextures[Input.SpriteIndex].Sample(p_Sampler_PointWrap, Input.TexCoord);
	clip(Color.a - 0.1f);
	return g_SpriteData[Input.SpriteIndex].Color * Color;
}
