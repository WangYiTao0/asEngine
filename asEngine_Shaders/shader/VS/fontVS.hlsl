#include "globals.hlsli"
#include "ShaderInterop_Font.h"

struct VertextoPixel
{
	float4 pos				: SV_POSITION;
	float2 tex				: TEXCOORD0;
};

VertextoPixel main(int2 inPos : POSITION, float2 inTex : TEXCOORD0)
{
	VertextoPixel Out;

	Out.pos = mul(g_xFont_Transform, float4(float2(inPos), 0, 1));
	
	Out.tex = inTex;

	return Out;
}
