#ifndef COMMON_PACKING_H
#define COMMON_PACKING_H

float2 UnpackHalf2(uint Packed)
{
	float2 Unpacked;
	Unpacked.x = f16tof32(Packed.x);
	Unpacked.y = f16tof32(Packed.x >> 16);
	return Unpacked;
}

uint PackHalf2(float2 Unpacked)
{
	uint Packed;
	Packed.x = f32tof16(Unpacked.x);
	Packed.x |= f32tof16(Unpacked.y) << 16;
	return Packed;
}

//

float3 UnpackHalf3(uint2 Packed)
{
	float3 Unpacked;
	Unpacked.x = f16tof32(Packed.x);
	Unpacked.y = f16tof32(Packed.x >> 16);
	Unpacked.z = f16tof32(Packed.y);
	return Unpacked;
}

uint2 PackHalf3(float3 Unpacked)
{
	uint2 Packed;
	Packed.x = f32tof16(Unpacked.x);
	Packed.x |= f32tof16(Unpacked.y) << 16;
	Packed.y = f32tof16(Unpacked.z);
	return Packed;
}

//

float4 UnpackHalf4(uint2 Packed)
{
	float4 Unpacked;
	Unpacked.x = f16tof32(Packed.x);
	Unpacked.y = f16tof32(Packed.x >> 16);
	Unpacked.z = f16tof32(Packed.y);
	Unpacked.w = f16tof32(Packed.y >> 16);
	return Unpacked;
}

uint2 PackHalf4(float4 Unpacked)
{
	uint2 Packed;
	Packed.x = f32tof16(Unpacked.x);
	Packed.x |= f32tof16(Unpacked.y) << 16;
	Packed.y = f32tof16(Unpacked.z);
	Packed.y |= f32tof16(Unpacked.w) << 16;
	return Packed;
}

//

float4 UnpackUintColor(uint color)
{
	return float4((color >> 24 & 0xff) / 255.0, (color >> 16 & 0xff) / 255.0f, (color >> 8 & 0xff) / 255.0, (color >> 0 & 0xff) / 255.0);
}

// http://aras-p.info/texts/CompactNormalStorage.html
// Spheremap Transform by Martin Mittring
half4 Spheremap_Encode(half3 n, float3 view)
{
	half p = sqrt(n.z * 8 + 8);
	return half4(n.xy / p + 0.5, 0, 0);
}

// Spheremap Transform by Martin Mittring
half3 Spheremap_Decode(half2 enc, float3 view)
{
	half2 fenc = enc * 4 - 2;
	half f = dot(fenc, fenc);
	half g = sqrt(1 - f / 4);
	half3 n;
	n.xy = fenc * g;
	n.z = 1 - f / 2;
	return n;
}

#endif