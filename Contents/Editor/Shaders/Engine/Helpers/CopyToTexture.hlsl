
struct VSInput
{
	uint Id : SV_VertexID;
};

struct PSInput
{
	float4 Position : SV_Position;
	float2 TexCoord : TexCoord;
};

// --------------------
// Vertex Shader
// --------------------

PSInput VS_Main(VSInput Vs)
{
	int2 TexCoord = int2(Vs.Id & 1, Vs.Id >> 1);
	
	PSInput Ps;
	Ps.Position = float4(2.f * (TexCoord.x - .5f), -2.f * (TexCoord.y - .5f), 0.f, 1.f);
	Ps.TexCoord = float2(TexCoord);
	return Ps;
}

// --------------------
// Pixel Shader
// --------------------

Texture2D<float4> p_CopySource : register(t0, space0);

SamplerState p_Sampler_PointWrap : register(s0, space0);
SamplerState p_Sampler_PointClamp : register(s1, space0);
SamplerState p_Sampler_LinearWrap : register(s2, space0);
SamplerState p_Sampler_LinearClamp : register(s3, space0);
SamplerState p_Sampler_AnisotropicWrap : register(s4, space0);
SamplerState p_Sampler_AnisotropicClamp : register(s5, space0);

float4 PS_Main(PSInput Ps) : SV_Target0
{
	return p_CopySource.Sample(p_Sampler_PointWrap, Ps.TexCoord);
}
