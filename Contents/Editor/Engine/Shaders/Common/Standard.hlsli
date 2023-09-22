#ifndef COMMON_STANDARD_ROOTSIGNATURE_H
#define COMMON_STANDARD_ROOTSIGNATURE_H

//

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

Texture2DMS<int> g_Texture2DMS_1i[] : register(t0, space94);
Texture2DMS<int2> g_Texture2DMS_2i[] : register(t0, space95);
Texture2DMS<int3> g_Texture2DMS_3i[] : register(t0, space96);
Texture2DMS<int4> g_Texture2DMS_4i[] : register(t0, space97);

Texture2DMS<uint> g_Texture2DMS_1u[] : register(t0, space98);
Texture2DMS<uint2> g_Texture2DMS_2u[] : register(t0, space99);
Texture2DMS<uint3> g_Texture2DMS_3u[] : register(t0, space100);
Texture2DMS<uint4> g_Texture2DMS_4u[] : register(t0, space101);


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

//
//
//
	

struct DescriptorTable
{
	template<
	typename T>
	static T LoadObject_R(uint Index, uint Address = 0)
	{
		return g_Buffer[Index].Load < T > (Address);
	}
	template<
	typename T>
	static T LoadObject_RW(uint Index, uint Address = 0)
	{
		return g_RW_Buffer[Index].Load < T > (Address);
	}
	template<
	typename T>
	static void StoreObject(uint Index, in T Object, uint Address = 0)
	{
		g_RW_Buffer[Index].Store < T > (Address, Object);
	}
	
	template<
	typename T>
	static T LoadTexture1D_R(uint Index, uint Address = 0)
	{
		return g_Texture1D[Index].Load < T > (Address);
	}
	
	//
	
	static SamplerState LoadSampler(uint Index)
	{
		return g_Samplers[Index];
	}
		
	static SamplerComparisonState LoadSamplerCmpState(uint Index)
	{
		return g_SamplersCmpState[Index];
	}
	
	//
	
	static Texture1D Load1D(uint Index)
	{
		return g_Texture1D[Index];
	}
	
	static Texture2D Load2D(uint Index)
	{
		return g_Texture2D[Index];
	}
	
	static Texture3D Load3D(uint Index)
	{
		return g_Texture3D[Index];
	}
	
	static TextureCube LoadCube(uint Index)
	{
		return g_TextureCube[Index];
	}
	
	//
	
	static RWTexture1D<float> Load1D_1f_RW(uint Index)
	{
		return g_RW_Texture1D_1f[Index];
	}
	static RWTexture1D<float2> Load1D_2f_RW(uint Index)
	{
		return g_RW_Texture1D_2f[Index];
	}
	static RWTexture1D<float3> Load1D_3f_RW(uint Index)
	{
		return g_RW_Texture1D_3f[Index];
	}
	static RWTexture1D<float4> Load1D_4f_RW(uint Index)
	{
		return g_RW_Texture1D_4f[Index];
	}
	static RWTexture1D<int> Load1D_1i_RW(uint Index)
	{
		return g_RW_Texture1D_1i[Index];
	}
	static RWTexture1D<int2> Load1D_2i_RW(uint Index)
	{
		return g_RW_Texture1D_2i[Index];
	}
	static RWTexture1D<int3> Load1D_3i_RW(uint Index)
	{
		return g_RW_Texture1D_3i[Index];
	}
	static RWTexture1D<int4> Load1D_4i_RW(uint Index)
	{
		return g_RW_Texture1D_4i[Index];
	}
	static RWTexture1D<uint> Load1D_1u_RW(uint Index)
	{
		return g_RW_Texture1D_1u[Index];
	}
	static RWTexture1D<uint2> Load1D_2u_RW(uint Index)
	{
		return g_RW_Texture1D_2u[Index];
	}
	static RWTexture1D<uint3> Load1D_3u_RW(uint Index)
	{
		return g_RW_Texture1D_3u[Index];
	}
	static RWTexture1D<uint4> Load1D_4u_RW(uint Index)
	{
		return g_RW_Texture1D_4u[Index];
	}
	
	//
	
	static RWTexture2D<float> Load2D_1f_RW(uint Index)
	{
		return g_RW_Texture2D_1f[Index];
	}
	static RWTexture2D<float2> Load2D_2f_RW(uint Index)
	{
		return g_RW_Texture2D_2f[Index];
	}
	static RWTexture2D<float3> Load2D_3f_RW(uint Index)
	{
		return g_RW_Texture2D_3f[Index];
	}
	static RWTexture2D<float4> Load2D_4f_RW(uint Index)
	{
		return g_RW_Texture2D_4f[Index];
	}
	static RWTexture2D<int> Load2D_1i_RW(uint Index)
	{
		return g_RW_Texture2D_1i[Index];
	}
	static RWTexture2D<int2> Load2D_2i_RW(uint Index)
	{
		return g_RW_Texture2D_2i[Index];
	}
	static RWTexture2D<int3> Load2D_3i_RW(uint Index)
	{
		return g_RW_Texture2D_3i[Index];
	}
	static RWTexture2D<int4> Load2D_4i_RW(uint Index)
	{
		return g_RW_Texture2D_4i[Index];
	}
	static RWTexture2D<uint> Load2D_1u_RW(uint Index)
	{
		return g_RW_Texture2D_1u[Index];
	}
	static RWTexture2D<uint2> Load2D_2u_RW(uint Index)
	{
		return g_RW_Texture2D_2u[Index];
	}
	static RWTexture2D<uint3> Load2D_3u_RW(uint Index)
	{
		return g_RW_Texture2D_3u[Index];
	}
	static RWTexture2D<uint4> Load2D_4u_RW(uint Index)
	{
		return g_RW_Texture2D_4u[Index];
	}
	
	//
	
	static RWTexture3D<float> Load3D_1f_RW(uint Index)
	{
		return g_RW_Texture3D_1f[Index];
	}
	static RWTexture3D<float2> Load3D_2f_RW(uint Index)
	{
		return g_RW_Texture3D_2f[Index];
	}
	static RWTexture3D<float3> Load3D_3f_RW(uint Index)
	{
		return g_RW_Texture3D_3f[Index];
	}
	static RWTexture3D<float4> Load3D_4f_RW(uint Index)
	{
		return g_RW_Texture3D_4f[Index];
	}
	static RWTexture3D<int> Load3D_1i_RW(uint Index)
	{
		return g_RW_Texture3D_1i[Index];
	}
	static RWTexture3D<int2> Load3D_2i_RW(uint Index)
	{
		return g_RW_Texture3D_2i[Index];
	}
	static RWTexture3D<int3> Load3D_3i_RW(uint Index)
	{
		return g_RW_Texture3D_3i[Index];
	}
	static RWTexture3D<int4> Load3D_4i_RW(uint Index)
	{
		return g_RW_Texture3D_4i[Index];
	}
	static RWTexture3D<uint> Load3D_1u_RW(uint Index)
	{
		return g_RW_Texture3D_1u[Index];
	}
	static RWTexture3D<uint2> Load3D_2u_RW(uint Index)
	{
		return g_RW_Texture3D_2u[Index];
	}
	static RWTexture3D<uint3> Load3D_3u_RW(uint Index)
	{
		return g_RW_Texture3D_3u[Index];
	}
	static RWTexture3D<uint4> Load3D_4u_RW(uint Index)
	{
		return g_RW_Texture3D_4u[Index];
	}
	
	//
	
	static Texture2DMS<float> LoadMS_1f_RW(uint Index)
	{
		return g_Texture2DMS_1f[Index];
	}
	static Texture2DMS<float2> LoadMS_2f_RW(uint Index)
	{
		return g_Texture2DMS_2f[Index];
	}
	static Texture2DMS<float3> LoadMS_3f_RW(uint Index)
	{
		return g_Texture2DMS_3f[Index];
	}
	static Texture2DMS<float4> LoadMS_4f_RW(uint Index)
	{
		return g_Texture2DMS_4f[Index];
	}
	static Texture2DMS<int> LoadMS_1i_RW(uint Index)
	{
		return g_Texture2DMS_1i[Index];
	}
	static Texture2DMS<int2> LoadMS_2i_RW(uint Index)
	{
		return g_Texture2DMS_2i[Index];
	}
	static Texture2DMS<int3> LoadMS_3i_RW(uint Index)
	{
		return g_Texture2DMS_3i[Index];
	}
	static Texture2DMS<int4> LoadMS_4i_RW(uint Index)
	{
		return g_Texture2DMS_4i[Index];
	}
	static Texture2DMS<uint> LoadMS_1u_RW(uint Index)
	{
		return g_Texture2DMS_1u[Index];
	}
	static Texture2DMS<uint2> LoadMS_2u_RW(uint Index)
	{
		return g_Texture2DMS_2u[Index];
	}
	static Texture2DMS<uint3> LoadMS_3u_RW(uint Index)
	{
		return g_Texture2DMS_3u[Index];
	}
	static Texture2DMS<uint4> LoadMS_4u_RW(uint Index)
	{
		return g_Texture2DMS_4u[Index];
	}
};


#endif