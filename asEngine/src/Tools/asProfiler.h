#pragma once
#include "Graphics\API\asGraphicsDevice.h"
#include "Helpers\asHashString.h"

#include <string>

namespace asProfiler
{
	typedef size_t range_id;

	// Begin collecting profiling data for the current frame
	void BeginFrame();

	// Finalize collecting profiling data for the current frame
	void EndFrame(asGraphics::CommandList cmd);

	// Start a CPU profiling range
	range_id BeginRangeCPU(const asHashString& name);

	// Start a GPU profiling range
	range_id BeginRangeGPU(const asHashString& name, asGraphics::CommandList cmd);

	// End a profiling range
	void EndRange(range_id id);

	// Renders a basic text of the Profiling results to the (x,y) screen coordinate
	void DrawData(int x, int y, asGraphics::CommandList cmd);

	// Enable/disable profiling
	void SetEnabled(bool value);
}