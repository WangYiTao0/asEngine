#ifndef AS_EMITTEDPARTICLE_HF
#define AS_EMITTEDPARTICLE_HF

struct VertextoPixel
{
	float4 pos						: SV_POSITION;
	float4 pos2D					: TEXCOORD0;
	float2 tex						: TEXCOORD1;
	nointerpolation float size		: TEXCOORD2;
	nointerpolation uint color		: TEXCOORD3;
};

#endif // AS_EMITTEDPARTICLE_HF
