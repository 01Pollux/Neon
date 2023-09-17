
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

static const int c_BlurRadius = 4;
static const int c_BlurSize = (c_BlurRadius * 2 + 1);
static const int c_CacheSize = (CS_KERNEL_SIZE + c_BlurRadius * 2);

//

struct BlurParams
{
	float w0,
		w1,
		w2,
		w3,
		w4,
		w5,
		w6,
		w7,
		w8;
};

ConstantBuffer<BlurParams> c_BlurParams : register(b0, space1);

Texture2D<float4> c_Input : register(t0, space1);
RWTexture2D<float4> c_Output : register(u0, space1);

//

groupshared float4 s_TempWeigths[c_CacheSize];

// --------------------
// Compute Shader
// --------------------


#ifdef BLUR_H
[numthreads(CS_KERNEL_SIZE, 1, 1)]
#else
[numthreads(1, CS_KERNEL_SIZE, 1)]
#endif
void CS_Main(
	int3 GTID : SV_GroupThreadID,
	int3 DTID : SV_DispatchThreadID)
{
	const float Weights[] =
	{
		c_BlurParams.w0,
		c_BlurParams.w1,
		c_BlurParams.w2,
		c_BlurParams.w3,
		c_BlurParams.w4,
		c_BlurParams.w5,
		c_BlurParams.w6,
		c_BlurParams.w7,
		c_BlurParams.w8
	};
	
	int2 DummySize;
	c_Input.GetDimensions(DummySize.x, DummySize.y);
	
#ifdef BLUR_H	
	int Size = DummySize.x;
	int GroupIndex = GTID.x;
	int DispatchIndex = DTID.x;
#else
	int Size = DummySize.y;
	int GroupIndex = GTID.y;
	int DispatchIndex = DTID.y;
#endif
	
	if (GroupIndex < c_BlurRadius)
	{
		int Index = max(DispatchIndex - c_BlurRadius, 0);
#ifdef BLUR_H
		s_TempWeigths[GroupIndex.x] = c_Input[int2(Index, DTID.y)];
#else
		s_TempWeigths[GroupIndex.x] = c_Input[int2(DTID.x, Index)];
#endif
	}
	else if (GroupIndex >= CS_KERNEL_SIZE - c_BlurRadius)
	{
		int Index = min(DispatchIndex + c_BlurRadius, Size - 1);
#ifdef BLUR_H
		s_TempWeigths[GroupIndex + 2 * c_BlurRadius] = c_Input[int2(Index, DTID.y)];
#else
		s_TempWeigths[GroupIndex + 2 * c_BlurRadius] = c_Input[int2(DTID.x, Index)];
#endif
	}

	s_TempWeigths[GroupIndex + c_BlurRadius] = c_Input[min(DTID.xy, DummySize - 1)];

	GroupMemoryBarrierWithGroupSync();
	
	float4 Blurred = (float4) 0.f;
	for (int i = 0; i < c_BlurSize; ++i)
	{
		Blurred += s_TempWeigths[GroupIndex + i] * Weights[i];
	}
	
	c_Output[DTID.xy] = Blurred;
}