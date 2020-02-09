#pragma once
#include "RenderPath3D.h"

namespace as
{
	class RenderPath3D_Forward :
		public RenderPath3D
	{
	protected:

		asGraphics::Texture rtMain[2];
		asGraphics::Texture rtMain_resolved[2];

		asGraphics::RenderPass renderpass_depthprepass;
		asGraphics::RenderPass renderpass_main;
		asGraphics::RenderPass renderpass_transparent;

		const constexpr asGraphics::Texture* GetSceneRT_Read(int i) const
		{
			if (getMSAASampleCount() > 1)
			{
				return &rtMain_resolved[i];
			}
			else
			{
				return &rtMain[i];
			}
		}

		void ResizeBuffers() override;

	public:
		void Render() const override;
	};

}