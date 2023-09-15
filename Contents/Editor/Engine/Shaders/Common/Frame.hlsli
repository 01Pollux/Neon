#ifndef COMMON_FRAME_H
#define COMMON_FRAME_H

struct PerFrameData
{
	matrix World;
	
	matrix View;
	matrix Projection;
	matrix ViewProjection;
	
	matrix ViewInverse;
	matrix ProjectionInverse;
	matrix ViewProjectionInverse;
	
	float2 ScreenResolution;
	
	float EngineTime;
	float GameTime;
	float DeltaTime;
};

#ifndef FRAME_DATA_OVERRIDE
ConstantBuffer<PerFrameData> g_FrameData : register(b0, space0);
#endif

#endif // COMMON_FRAME_H