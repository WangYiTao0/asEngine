#include "../HF/deferredLightHF.hlsli"
#include "../HF/cone.hlsli"


VertexToPixel main(uint vid : SV_VERTEXID)
{
	VertexToPixel Out;
		
	float4 pos = CONE[vid];
	Out.pos = Out.pos2D = mul(g_xTransform, pos);
	return Out;
}