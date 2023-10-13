#ifndef COMMON_STANDARD_H
#define COMMON_STANDARD_H

// Space 48 -> 63: R/W Buffers 

SamplerState g_Samplers[] : register(s0, space48);
SamplerComparisonState g_SamplersCmpState[] : register(s0, space48);

// Space 48 -> 63: R/W Buffers 

RWByteAddressBuffer g_RW_Buffer[] : register(u0, space48);

ByteAddressBuffer g_Buffer[] : register(t0, space49);

// Space 64 -> 76: R/W Textures 1D

RWTexture1D<float> g_RW_Texture1D_1f[] : register(u0, space64);
RWTexture1D<float2> g_RW_Texture1D_2f[] : register(u0, space65);
RWTexture1D<float3> g_RW_Texture1D_3f[] : register(u0, space66);
RWTexture1D<float4> g_RW_Texture1D_4f[] : register(u0, space67);

RWTexture1D<int> g_RW_Texture1D_1i[] : register(u0, space68);
RWTexture1D<int2> g_RW_Texture1D_2i[] : register(u0, space69);
RWTexture1D<int3> g_RW_Texture1D_3i[] : register(u0, space70);
RWTexture1D<int4> g_RW_Texture1D_4i[] : register(u0, space71);

RWTexture1D<uint> g_RW_Texture1D_1u[] : register(u0, space72);
RWTexture1D<uint2> g_RW_Texture1D_2u[] : register(u0, space73);
RWTexture1D<uint3> g_RW_Texture1D_3u[] : register(u0, space74);
RWTexture1D<uint4> g_RW_Texture1D_4u[] : register(u0, space75);

Texture1D g_Texture1D[] : register(t0, space76);

// Space 77 -> 91: R/W Textures 2D

RWTexture2D<float> g_RW_Texture2D_1f[] : register(u0, space77);
RWTexture2D<float2> g_RW_Texture2D_2f[] : register(u0, space78);
RWTexture2D<float3> g_RW_Texture2D_3f[] : register(u0, space79);
RWTexture2D<float4> g_RW_Texture2D_4f[] : register(u0, space80);

RWTexture2D<int> g_RW_Texture2D_1i[] : register(u0, space81);
RWTexture2D<int2> g_RW_Texture2D_2i[] : register(u0, space82);
RWTexture2D<int3> g_RW_Texture2D_3i[] : register(u6, space83);
RWTexture2D<int4> g_RW_Texture2D_4i[] : register(u7, space84);

RWTexture2D<uint> g_RW_Texture2D_1u[] : register(u8, space85);
RWTexture2D<uint2> g_RW_Texture2D_2u[] : register(u9, space86);
RWTexture2D<uint3> g_RW_Texture2D_3u[] : register(u0, space87);
RWTexture2D<uint4> g_RW_Texture2D_4u[] : register(u0, space88);

Texture2D g_Texture2D[] : register(t0, space89);

// Space 92 -> 103: R/W Textures 2D MS

Texture2DMS<float> g_Texture2DMS_1f[] : register(t0, space90);
Texture2DMS<float2> g_Texture2DMS_2f[] : register(t0, space91);
Texture2DMS<float3> g_Texture2DMS_3f[] : register(t0, space92);
Texture2DMS<float4> g_Texture2DMS_4f[] : register(t0, space93);

// Space 104 -> 116: R/W Textures 3D

RWTexture3D<float> g_RW_Texture3D_1f[] : register(u0, space102);
RWTexture3D<float2> g_RW_Texture3D_2f[] : register(u0, space103);
RWTexture3D<float3> g_RW_Texture3D_3f[] : register(u0, space104);
RWTexture3D<float4> g_RW_Texture3D_4f[] : register(u0, space105);

RWTexture3D<int> g_RW_Texture3D_1i[] : register(u0, space106);
RWTexture3D<int2> g_RW_Texture3D_2i[] : register(u0, space107);
RWTexture3D<int3> g_RW_Texture3D_3i[] : register(u6, space108);
RWTexture3D<int4> g_RW_Texture3D_4i[] : register(u7, space109);

RWTexture3D<uint> g_RW_Texture3D_1u[] : register(u8, space110);
RWTexture3D<uint2> g_RW_Texture3D_2u[] : register(u9, space111);
RWTexture3D<uint3> g_RW_Texture3D_3u[] : register(u0, space112);
RWTexture3D<uint4> g_RW_Texture3D_4u[] : register(u0, space113);

Texture3D g_Texture3D[] : register(t0, space114);

// Space 117 Textures Cube

TextureCube g_TextureCube[] : register(t0, space115);

#endif