#pragma once

#include "ShaderInterop.h"

CBUFFER(ImageCB, CBSLOT_IMAGE)
{
	float4		xCorners[4];
	float4		xTexMulAdd;
	float4		xTexMulAdd2;
	float4		xColor;
	float2		padding_imageCB;
	uint		xMirror;
	float		xMipLevel;
};

