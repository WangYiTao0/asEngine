#ifndef AS_SHADERINTEROP_FONT_H
#define AS_SHADERINTEROP_FONT_H
#include "ShaderInterop.h"

CBUFFER(FontCB, CBSLOT_FONT)
{
	float4x4	g_xFont_Transform;
	float4		g_xFont_Color;
};

#endif // AS_SHADERINTEROP_FONT_H
