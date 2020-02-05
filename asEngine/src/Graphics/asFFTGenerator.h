#pragma once

#include "CommonInclude.h"
#include "Graphics/API/asGraphicsDevice.h"

namespace asFFTGenerator
{
	struct CSFFT512x512_Plan
	{
		// More than one array can be transformed at same time
		uint32_t slices;

		// For 512x512 config, we need 6 constant buffers
		asGraphics::GPUBuffer pRadix008A_CB[6];

		asGraphics::GPUBuffer pBuffer_Tmp;
	};


	void fft512x512_create_plan(CSFFT512x512_Plan& plan, uint32_t slices);

	void fft_512x512_c2c(
		const CSFFT512x512_Plan& fft_plan,
		const asGraphics::GPUResource& pUAV_Dst,
		const asGraphics::GPUResource& pSRV_Dst,
		const asGraphics::GPUResource& pSRV_Src,
		asGraphics::CommandList cmd);

	void LoadShaders();
}