#ifndef AS_IMPOSTOR_HF
#define AS_IMPOSTOR_HF

struct VSOut
{
	float4 pos						: SV_POSITION;
	float3 tex						: TEXCOORD;
	nointerpolation float dither	: DITHER;
	float3 pos3D					: WORLDPOSITION;
	uint instanceColor				: INSTANCECOLOR;
	float4 pos2D					: SCREENPOSITION;
	float4 pos2DPrev				: SCREENPOSITIONPREV;
};

TEXTURE2DARRAY(impostorTex, float4, TEXSLOT_ONDEMAND0);

#endif // AS_IMPOSTOR_HF
