#pragma once
#include "RenderPath3D.h"

namespace as
{
	class RenderPath3D_PathTracing :
		public RenderPath3D
	{
	private:
		int sam = -1;

	protected:
		asGraphics::Texture traceResult;

		asGraphics::RenderPass renderpass_debugbvh;

		void ResizeBuffers() override;

	public:
		const asGraphics::Texture* GetDepthStencil() const override { return nullptr; };

		void Update(float dt) override;
		void Render() const override;
		void Compose(asGraphics::CommandList cmd) const override;
	};

}