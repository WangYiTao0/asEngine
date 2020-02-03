#pragma once

#include "ShaderInterop.h"

CBUFFER(SortConstants, CBSLOT_OTHER_GPUSORTLIB)
{
	int3 job_params;
	uint counterReadOffset;
};
