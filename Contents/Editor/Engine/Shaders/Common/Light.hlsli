#ifndef COMMON_LIGHT_H
#define COMMON_LIGHT_H

#define MAX_LIGHTS 1024

#define LIGHT_FLAGS_TYPE_DIRECTIONAL (1 << 0)
#define LIGHT_FLAGS_TYPE_POINT (1 << 1)
#define LIGHT_FLAGS_TYPE_SPOT_LIGHT (1 << 2)
#define LIGHT_FLAGS_TYPE_MASK (LIGHT_FLAGS_TYPE_DIRECTIONAL | LIGHT_FLAGS_TYPE_POINT | LIGHT_FLAGS_TYPE_SPOT_LIGHT)

#define LIGHT_FLAGS_ENABLED (1 << 3)

struct Light
{
	float4 Color;
	float3 Position;
	float Range;
	float3 Direction;
	uint Flags;
	float3 Attenuation_Angle;
    
    //
    
	bool IsEnabled()
	{
		return (Flags & LIGHT_FLAGS_ENABLED) != 0;
	}
    
	uint GetType()
	{
		return (Flags & LIGHT_FLAGS_TYPE_MASK);
	}
	
	//
	
	float3 Point_GetAttenuation()
	{
		return Attenuation_Angle.x;
	}
	
	//
	
	float Spot_GetAttenuation()
	{
		return Attenuation_Angle.x;
	}
	
	float Spot_GetAngle()
	{
		return Attenuation_Angle.y;
	}
	
	float Spot_GetAngleAttenuation()
	{
		return Attenuation_Angle.z;
	}
};

#endif