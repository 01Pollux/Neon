
// --------------------
// Structures
// --------------------
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


//

struct VSInput
{
	float3 Position : Position;
	float4 Color : Color;
	int NeedsProjection : NeedsProjection;
};


struct PSInput
{
	float4 Position : SV_Position;
	float4 Color : Color;
};

// --------------------
// Global
// --------------------

ConstantBuffer<PerFrameData> g_FrameData : register(b0, space1);

// --------------------
// Vertex Shader
// --------------------

PSInput VS_Main(VSInput Vs)
{
	PSInput Ps;
	
	Ps.Position = float4(Vs.Position, 1.0f);
	if (Vs.NeedsProjection)
	{
		Ps.Position = mul(Ps.Position, g_FrameData.ViewProjection);
	}
	Ps.Color = Vs.Color;
	
	return Ps;
}

// --------------------
// Pixel Shader
// --------------------

float4 PS_Main(PSInput Ps) : SV_TARGET
{
	return Ps.Color;
}
