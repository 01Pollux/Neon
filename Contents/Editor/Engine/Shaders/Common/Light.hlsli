#ifndef COMMON_LIGHT_H
#define COMMON_LIGHT_H

#define MAX_LIGHTS 1024
#define LIGHT_CLUSTER_SIZE 16

#define LIGHT_FLAGS_TYPE_DIRECTIONAL (1 << 0)
#define LIGHT_FLAGS_TYPE_POINT (1 << 1)
#define LIGHT_FLAGS_TYPE_SPOT_LIGHT (1 << 2)
#define LIGHT_FLAGS_TYPE_MASK (LIGHT_FLAGS_TYPE_DIRECTIONAL | LIGHT_FLAGS_TYPE_POINT | LIGHT_FLAGS_TYPE_SPOT_LIGHT)

#define LIGHT_FLAGS_ENABLED (1 << 3)

struct LightingResult
{
	float4 Diffuse;
	float4 Specular;
};

//

static float4 Light_Diffuse(in const float4 Color, in const float4 L, in const float4 N)
{
	float NdotL = max(dot(L, N), 0);
	return Color * NdotL;
}

static float4 Light_DoSpecular(in const float4 Color, in const float SpecularPower, float4 V, float4 L, float4 N)
{
	float4 R = normalize(reflect(-L, N));
	float RdotV = max(dot(R, V), 0);

	return Color * pow(RdotV, SpecularPower);
}

//

struct Light
{
	float4 Color;
	float3 Position;
	float Range;
	float3 Direction;
	uint Flags;
	float3 Attenuation_Angle;
	float _Pad;
    
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
	
	//
	
	void Process(in const float4 TargetToEye, in const float4 TargetPosition, in const float4 Normal, inout LightingResult Result)
	{
		switch (GetType())
		{
			case LIGHT_FLAGS_TYPE_DIRECTIONAL:
				{
					Result.Diffuse += Light_Diffuse(Color, float4(-Direction, 0.f), Normal);
					break;
				}
		}

	}
};

#endif