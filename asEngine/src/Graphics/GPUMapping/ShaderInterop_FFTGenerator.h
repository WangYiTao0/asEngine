#ifndef AS_SHADERINTEROP_FFTGENERATOR_H
#define AS_SHADERINTEROP_FFTGENERATOR_H

#include "ShaderInterop.h"

#define COHERENCY_GRANULARITY 128

CBUFFER(FFTGeneratorCB, CBSLOT_OTHER_FFTGENERATOR)
{
	uint thread_count;
	uint ostride;
	uint istride;
	uint pstride;

	float phase_base;
};


#endif // AS_SHADERINTEROP_FFTGENERATOR_H
