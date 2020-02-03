#pragma once

#include "ShaderInterop.h"

CBUFFER(FontCB, CBSLOT_FONT)
{
	float4x4	g_xFont_Transform;
	float4		g_xFont_Color;
};


