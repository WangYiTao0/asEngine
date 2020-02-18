#include "../HF/globals.hlsli"

float4 main(float4 inPos : POSITION_NORMAL_SUBSETINDEX) : SV_POSITION
{
	float4 pos = mul(g_xTransform, float4(inPos.xyz, 1));

	return pos;
}