#pragma once
#include "RenderPath3D.h"

namespace as
{
	class RenderPath3D_Deferred :
		public RenderPath3D
	{
	protected:
		asGraphics::Texture rtGBuffer[3];
		asGraphics::Texture rtDeferred;
		asGraphics::Texture rtSSS[2];
		asGraphics::Texture lightbuffer_diffuse;
		asGraphics::Texture lightbuffer_specular;

		asGraphics::RenderPass renderpass_gbuffer;
		asGraphics::RenderPass renderpass_lights;
		asGraphics::RenderPass renderpass_decals;
		asGraphics::RenderPass renderpass_deferredcomposition;
		asGraphics::RenderPass renderpass_SSS[3];
		asGraphics::RenderPass renderpass_transparent;
		//asGraphics::RenderPass renderPass_

		void ResizeBuffers() override;

		virtual void RenderSSS(asGraphics::CommandList cmd) const;
		virtual void RenderDecals(asGraphics::CommandList cmd) const;
		virtual void RenderDeferredComposition(asGraphics::CommandList cmd) const;

		virtual void RenderDebugImage(asGraphics::CommandList cmd)const;
	public:
		void setMSAASampleCount(uint32_t value) override { /*disable MSAA for deferred*/ }

		void Render() const override;
		
	};
}

