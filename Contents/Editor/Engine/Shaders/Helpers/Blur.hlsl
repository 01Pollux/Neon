
#ifndef CS_KERNEL_SIZE_X
#define CS_KERNEL_SIZE_X 16
#endif

#ifndef CS_KERNEL_SIZE_Y
#define CS_KERNEL_SIZE_Y 16
#endif


#ifdef BLUR_H
#define CS_KERNEL_SIZE CS_KERNEL_SIZE_X
#else
#define CS_KERNEL_SIZE CS_KERNEL_SIZE_Y
#endif

#define BLUR_RADIUS 4

#define BLUR_SIZE (BLUR_RADIUS * 2 + 1)

#define CACHE_SIZE (CS_KERNEL_SIZE + BLUR_RADIUS * 2)

//

struct BlurParams
{
	float Weights[BLUR_SIZE];
};

ConstantBuffer<BlurParams> c_BlurParams : register(b0, space1);

Texture2D<float4> c_Input : register(t0, space1);
RWTexture2D<float4> c_Output : register(u0, space1);

//

groupshared float4 s_TempWeigths[CACHE_SIZE];

// --------------------
// Compute Shader
// --------------------

#ifdef BLUR_H
[numthreads(CS_KERNEL_SIZE, 1, 1)]
#else
[numthreads(1, CS_KERNEL_SIZE, 1)]
#endif
void CS_Main(
	uint GID : SV_GroupID,
	uint3 GTID : SV_GroupThreadID,
	uint3 DTID : SV_DispatchThreadID)
{
#if BLUR_H
	uint GroupIndex = GTID.x;
#else
	uint GroupIndex = GTID.y;
#endif
	
	float4 Col = c_Input[DTID.xy];
	s_TempWeigths[GroupIndex + BLUR_RADIUS] = Col;
	
	// We are on the edge of the image, so we need to fetch the pixels from the other side of the image
	if (GID == 0)
	{
		for (uint i = 0; i < BLUR_RADIUS; ++i)
		{
#if BLUR_H
			s_TempWeigths[i] = c_Input[DTID.xy + int2(-BLUR_RADIUS + i, 0)];
#else
			s_TempWeigths[i] = c_Input[DTID.xy + int2(0, -BLUR_RADIUS + i)];
#endif
		}
	}
	else if (GID == CS_KERNEL_SIZE - 1)
	{
		for (uint i = 0; i < BLUR_RADIUS; ++i)
		{
#if BLUR_H
			s_TempWeigths[CS_KERNEL_SIZE + BLUR_RADIUS + i] = c_Input[DTID.xy + int2(i + 1, 0)];
#else
			s_TempWeigths[CS_KERNEL_SIZE + BLUR_RADIUS + i] = c_Input[DTID.xy + int2(0, i + 1)];
#endif
		}
	}
	
	GroupMemoryBarrierWithGroupSync();
	
	float4 Result = (float4) 0.f;
	for (uint i = 0; i < BLUR_SIZE; ++i)
	{
		Result += s_TempWeigths[GroupIndex + i] * c_BlurParams.Weights[i];
	}
	
	c_Output[DTID.xy] = Result;
}