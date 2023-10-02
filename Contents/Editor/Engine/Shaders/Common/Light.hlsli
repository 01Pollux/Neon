#ifndef COMMON_LIGHT_H
#define COMMON_LIGHT_H

#define LIGHT_FLAGS_ENABLED (1 << 0)
#define LIGHT_FLAGS_SELECTED (1 << 1)

#define LIGHT_FLAGS_TYPE_DIRECTIONAL (1 << 31)
#define LIGHT_FLAGS_TYPE_POINT (1 << 30)
#define LIGHT_FLAGS_TYPE_SPOT_LIGHT (1 << 29)
#define LIGHT_FLAGS_TYPE_MASK (LIGHT_FLAGS_TYPE_DIRECTIONAL | LIGHT_FLAGS_TYPE_POINT | LIGHT_FLAGS_TYPE_SPOT_LIGHT)

struct Light
{
    // Position for point and spot lights (View space).
	float3 PositionVS;
    
    // The half angle of the spotlight cone.
	float SpotlightAngle;
    
    // Direction for spot and directional lights (View space).
	float3 DirectionVS;
    
    // Color of the light. Diffuse and specular colors are not seperated.
	float4 Color;
    
    // The range of the light.
	float Range;

    // The intensity of the light.
	float Intensity;
    
    // LIGHT_FLAGS_*
	uint Flags;

    //
    
	bool IsEnabled()
	{
		return (Flags & LIGHT_FLAGS_ENABLED) != 0;
	}
    
	uint GetType()
	{
		return (Flags & LIGHT_FLAGS_TYPE_MASK);
	}
};

#endif