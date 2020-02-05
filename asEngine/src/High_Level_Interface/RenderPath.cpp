#include "aspch.h"
#include "RenderPath.h"
#include "Graphics\asRenderer.h"

void RenderPath::Update(float dt)
{
	if (asRenderer::ResolutionChanged() || !initial_resizebuffer)
	{
		ResizeBuffers();
		initial_resizebuffer = true;
	}
}

void RenderPath::Start()
{
	if (onStart != nullptr)
	{
		onStart();
	}
}

void RenderPath::Stop()
{
	if (onStop != nullptr)
	{
		onStop();
	}
}


