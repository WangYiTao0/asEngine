#pragma once
#include "RenderPath2D.h"
#include "Graphics/asRenderer.h"
#include "Graphics/API/asGraphicsDevice.h"
#include "Helpers/asResourceManager.h"

namespace as
{
	class RenderPath3D :
		public RenderPath2D
	{
	private:
		float exposure = 1.0f;
		float bloomThreshold = 1.0f;
		float ssaoBlur = 2.3f;
		float motionBlurStrength = 100.0f;
		float dofFocus = 10.0f;
		float dofStrength = 1.0f;
		float dofAspect = 1.0f;
		float sharpenFilterAmount = 0.28f;
		float outlineThreshold = 0.2f;
		float outlineThickness = 1.0f;
		XMFLOAT4 outlineColor = XMFLOAT4(0, 0, 0, 1);
		float ssaoRange = 1.0f;
		uint32_t ssaoSampleCount = 16;
		float ssaoPower = 2.0f;
		float chromaticAberrationAmount = 2.0f;

		bool fxaaEnabled = false;
		bool ssaoEnabled = false;
		bool ssrEnabled = false;
		bool reflectionsEnabled = true;
		bool shadowsEnabled = true;
		bool bloomEnabled = true;
		bool colorGradingEnabled = false;
		bool volumeLightsEnabled = true;
		bool lightShaftsEnabled = false;
		bool lensFlareEnabled = true;
		bool motionBlurEnabled = false;
		bool sssEnabled = true;
		bool depthOfFieldEnabled = false;
		bool eyeAdaptionEnabled = false;
		bool tessellationEnabled = false;
		bool sharpenFilterEnabled = false;
		bool outlineEnabled = false;
		bool chromaticAberrationEnabled = false;

		std::shared_ptr<asResource> colorGradingTex;

		uint32_t msaaSampleCount = 1;

	protected:
		asGraphics::Texture rtReflection; // conains the scene rendered for planar reflections
		asGraphics::Texture rtSSR; // screen-space reflection results
		asGraphics::Texture rtSceneCopy; // contains the rendered scene that can be fed into transparent pass for distortion effect
		asGraphics::Texture rtWaterRipple; // water ripple sprite normal maps are rendered into this
		asGraphics::Texture rtParticleDistortion; // contains distortive particles
		asGraphics::Texture rtParticleDistortion_Resolved; // contains distortive particles
		asGraphics::Texture rtVolumetricLights; // contains the volumetric light results
		asGraphics::Texture rtTemporalAA[2]; // temporal AA history buffer
		asGraphics::Texture rtBloom; // contains the bright parts of the image + mipchain
		asGraphics::Texture rtSSAO[2]; // ping-pong when rendering and blurring SSAO
		asGraphics::Texture rtSun[2]; // 0: sun render target used for lightshafts (can be MSAA), 1: radial blurred lightshafts
		asGraphics::Texture rtSun_resolved; // sun render target, but the resolved version if MSAA is enabled

		asGraphics::Texture rtPostprocess_HDR; // ping-pong with main scene RT in HDR post-process chain
		asGraphics::Texture rtPostprocess_LDR[2]; // ping-pong with itself in LDR post-process chain

		asGraphics::Texture depthBuffer; // used for depth-testing, can be MSAA
		asGraphics::Texture depthBuffer_Copy; // used for shader resource, single sample
		asGraphics::Texture depthBuffer_Reflection; // used for reflection, single sample
		asGraphics::Texture rtLinearDepth; // linear depth result
		asGraphics::Texture rtLinearDepth_minmax; // linear depth result (halfres minmax, mipchain)
		asGraphics::Texture smallDepth; // downsampled depth buffer

		asGraphics::RenderPass renderpass_occlusionculling;
		asGraphics::RenderPass renderpass_reflection;
		asGraphics::RenderPass renderpass_downsampledepthbuffer;
		asGraphics::RenderPass renderpass_lightshafts;
		asGraphics::RenderPass renderpass_volumetriclight;
		asGraphics::RenderPass renderpass_particledistortion;
		asGraphics::RenderPass renderpass_waterripples;

		// Post-processes are ping-ponged, this function helps to obtain the last postprocess render target that was written
		const asGraphics::Texture* GetLastPostprocessRT() const
		{
			int ldr_postprocess_count = 0;
			ldr_postprocess_count += sharpenFilterEnabled ? 1 : 0;
			ldr_postprocess_count += colorGradingEnabled ? 1 : 0;
			ldr_postprocess_count += fxaaEnabled ? 1 : 0;
			ldr_postprocess_count += chromaticAberrationEnabled ? 1 : 0;
			int rt_index = ldr_postprocess_count % 2;
			return &rtPostprocess_LDR[rt_index];
		}

		void ResizeBuffers() override;

		virtual void RenderFrameSetUp(asGraphics::CommandList cmd) const;
		virtual void RenderReflections(asGraphics::CommandList cmd) const;
		virtual void RenderShadows(asGraphics::CommandList cmd) const;

		virtual void RenderLinearDepth(asGraphics::CommandList cmd) const;
		virtual void RenderSSAO(asGraphics::CommandList cmd) const;
		virtual void RenderSSR(const asGraphics::Texture& srcSceneRT, const asGraphics::Texture& gbuffer1, asGraphics::CommandList cmd) const;
		virtual void DownsampleDepthBuffer(asGraphics::CommandList cmd) const;
		virtual void RenderOutline(const asGraphics::Texture& dstSceneRT, asGraphics::CommandList cmd) const;
		virtual void RenderLightShafts(asGraphics::CommandList cmd) const;
		virtual void RenderVolumetrics(asGraphics::CommandList cmd) const;
		virtual void RenderRefractionSource(const asGraphics::Texture& srcSceneRT, asGraphics::CommandList cmd) const;
		virtual void RenderTransparents(const asGraphics::RenderPass& renderpass_transparent, RENDERPASS renderPass, asGraphics::CommandList cmd) const;
		virtual void RenderPostprocessChain(const asGraphics::Texture& srcSceneRT, const asGraphics::Texture& srcGbuffer1, asGraphics::CommandList cmd) const;

	public:
		const asGraphics::Texture* GetDepthStencil() const override { return &depthBuffer; }

		constexpr float getExposure() const { return exposure; }
		constexpr float getBloomThreshold() const { return bloomThreshold; }
		constexpr float getSSAOBlur() const { return ssaoBlur; }
		constexpr float getMotionBlurStrength() const { return motionBlurStrength; }
		constexpr float getDepthOfFieldFocus() const { return dofFocus; }
		constexpr float getDepthOfFieldStrength() const { return dofStrength; }
		constexpr float getDepthOfFieldAspect() const { return dofAspect; }
		constexpr float getSharpenFilterAmount() const { return sharpenFilterAmount; }
		constexpr float getOutlineThreshold() const { return outlineThreshold; }
		constexpr float getOutlineThickness() const { return outlineThickness; }
		constexpr XMFLOAT4 getOutlineColor() const { return outlineColor; }
		constexpr float getSSAORange() const { return ssaoRange; }
		constexpr uint32_t getSSAOSampleCount() const { return ssaoSampleCount; }
		constexpr float getSSAOPower() const { return ssaoPower; }
		constexpr float getChromaticAberrationAmount() const { return chromaticAberrationAmount; }

		constexpr bool getSSAOEnabled() const { return ssaoEnabled; }
		constexpr bool getSSREnabled() const { return ssrEnabled; }
		constexpr bool getShadowsEnabled() const { return shadowsEnabled; }
		constexpr bool getReflectionsEnabled() const { return reflectionsEnabled; }
		constexpr bool getFXAAEnabled() const { return fxaaEnabled; }
		constexpr bool getBloomEnabled() const { return bloomEnabled; }
		constexpr bool getColorGradingEnabled() const { return colorGradingEnabled; }
		constexpr bool getVolumeLightsEnabled() const { return volumeLightsEnabled; }
		constexpr bool getLightShaftsEnabled() const { return lightShaftsEnabled; }
		constexpr bool getLensFlareEnabled() const { return lensFlareEnabled; }
		constexpr bool getMotionBlurEnabled() const { return motionBlurEnabled; }
		constexpr bool getSSSEnabled() const { return sssEnabled; }
		constexpr bool getDepthOfFieldEnabled() const { return depthOfFieldEnabled; }
		constexpr bool getEyeAdaptionEnabled() const { return eyeAdaptionEnabled; }
		constexpr bool getTessellationEnabled() const { return tessellationEnabled && asRenderer::GetDevice()->CheckCapability(asGraphics::GraphicsDevice::GRAPHICSDEVICE_CAPABILITY_TESSELLATION); }
		constexpr bool getSharpenFilterEnabled() const { return sharpenFilterEnabled && getSharpenFilterAmount() > 0; }
		constexpr bool getOutlineEnabled() const { return outlineEnabled; }
		constexpr bool getChromaticAberrationEnabled() const { return chromaticAberrationEnabled; }

		constexpr const std::shared_ptr<asResource>& getColorGradingTexture() const { return colorGradingTex; }

		constexpr uint32_t getMSAASampleCount() const { return msaaSampleCount; }

		constexpr void setExposure(float value) { exposure = value; }
		constexpr void setBloomThreshold(float value) { bloomThreshold = value; }
		constexpr void setSSAOBlur(float value) { ssaoBlur = value; }
		constexpr void setMotionBlurStrength(float value) { motionBlurStrength = value; }
		constexpr void setDepthOfFieldFocus(float value) { dofFocus = value; }
		constexpr void setDepthOfFieldStrength(float value) { dofStrength = value; }
		constexpr void setDepthOfFieldAspect(float value) { dofAspect = value; }
		constexpr void setSharpenFilterAmount(float value) { sharpenFilterAmount = value; }
		constexpr void setOutlineThreshold(float value) { outlineThreshold = value; }
		constexpr void setOutlineThickness(float value) { outlineThickness = value; }
		constexpr void setOutlineColor(const XMFLOAT4& value) { outlineColor = value; }
		constexpr void setSSAORange(float value) { ssaoRange = value; }
		constexpr void setSSAOSampleCount(uint32_t value) { ssaoSampleCount = value; }
		constexpr void setSSAOPower(float value) { ssaoPower = value; }
		constexpr void setChromaticAberrationAmount(float value) { chromaticAberrationAmount = value; }

		constexpr void setSSAOEnabled(bool value) { ssaoEnabled = value; }
		constexpr void setSSREnabled(bool value) { ssrEnabled = value; }
		constexpr void setShadowsEnabled(bool value) { shadowsEnabled = value; }
		constexpr void setReflectionsEnabled(bool value) { reflectionsEnabled = value; }
		constexpr void setFXAAEnabled(bool value) { fxaaEnabled = value; }
		constexpr void setBloomEnabled(bool value) { bloomEnabled = value; }
		constexpr void setColorGradingEnabled(bool value) { colorGradingEnabled = value; }
		constexpr void setVolumeLightsEnabled(bool value) { volumeLightsEnabled = value; }
		constexpr void setLightShaftsEnabled(bool value) { lightShaftsEnabled = value; }
		constexpr void setLensFlareEnabled(bool value) { lensFlareEnabled = value; }
		constexpr void setMotionBlurEnabled(bool value) { motionBlurEnabled = value; }
		constexpr void setSSSEnabled(bool value) { sssEnabled = value; }
		constexpr void setDepthOfFieldEnabled(bool value) { depthOfFieldEnabled = value; }
		constexpr void setEyeAdaptionEnabled(bool value) { eyeAdaptionEnabled = value; }
		constexpr void setTessellationEnabled(bool value) { tessellationEnabled = value; }
		constexpr void setSharpenFilterEnabled(bool value) { sharpenFilterEnabled = value; }
		constexpr void setOutlineEnabled(bool value) { outlineEnabled = value; }
		constexpr void setChromaticAberrationEnabled(bool value) { chromaticAberrationEnabled = value; }

		void setColorGradingTexture(std::shared_ptr<asResource> resource) { colorGradingTex = resource; }

		virtual void setMSAASampleCount(uint32_t value) { if (msaaSampleCount != value) { msaaSampleCount = value; ResizeBuffers(); } }

		void Update(float dt) override;
		void Render() const override = 0;
		void Compose(asGraphics::CommandList cmd) const override;
	};
}
