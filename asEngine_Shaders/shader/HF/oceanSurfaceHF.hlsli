#ifndef AS_OCEAN_SURFACE_HF
#define AS_OCEAN_SURFACE_HF
#include "globals.hlsli"
#include "ShaderInterop_Ocean.h"


struct PSIn
{
	float4 pos		: SV_POSITION;
	float4 pos2D	: SCREENPOSITION;
	float3 pos3D	: WORLDPOSITION;
	float2 uv		: TEXCOORD0;
	float4 ReflectionMapSamplingPos : REFLECTIONPOS;
};

#endif // AS_OCEAN_SURFACE_HF
