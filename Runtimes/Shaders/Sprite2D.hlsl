struct VSInput
{
	float3 Position : POSITION;
	float2 TexCoord : TEXCOORD;
	int TexIndex : TEXINDEX;
	float4 Color : COLOR;
};

struct VSOutput
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
	float2 TexCoord : TEXCOORD;
	nointerpolation int TexIndex : TEXINDEX;
};

VSOutput VS_Main(VSInput Input)
{
	VSOutput Output;
	Output.Position = float4(Input.Position, 1.0f);
	Output.TexCoord = Input.TexCoord;
	Output.Color = Input.Color;
	Output.TexIndex = Input.TexIndex;
	return Output;
}

Texture2D<float4> Texture : register(t0, space0);

SamplerState Sampler_PointWrap : register(s0, space0);
SamplerState Sampler_PointClamp : register(s1, space0);
SamplerState Sampler_LinearWrap : register(s2, space0);
SamplerState Sampler_LinearClamp : register(s3, space0);
SamplerState Sampler_AnisotropicWrap : register(s4, space0);
SamplerState Sampler_AnisotropicClamp : register(s5, space0);

float4 PS_Main(VSOutput Input) : SV_TARGET
{
	float4 Color = (float4) 1.f;
	if (Input.TexIndex != -1)
	{
		Color = Texture.Sample(Sampler_PointWrap, Input.TexCoord);
	}
	return Color * Input.Color;
}