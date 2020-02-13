#include <asEngine.h>
#include "RendererWindow.h"
#include "EditorComponent.h"

namespace as
{
	RendererWindow::RendererWindow(asGUI* gui, EditorComponent* editorcomponent, RenderPath3D* path) : GUI(gui)
	{
		assert(GUI && "Invalid GUI!");

		float screenW = (float)asRenderer::GetDevice()->GetScreenWidth();
		float screenH = (float)asRenderer::GetDevice()->GetScreenHeight();

		asRenderer::SetToDrawDebugEnvProbes(true);
		asRenderer::SetToDrawGridHelper(true);
		asRenderer::SetToDrawDebugCameras(true);

		rendererWindow = new asWindow(GUI, "Renderer Window");
		rendererWindow->SetSize(XMFLOAT2(640, 790));
		GUI->AddWidget(rendererWindow);

		float x = 260, y = 20, step = 30;

		vsyncCheckBox = new asCheckBox("VSync: ");
		vsyncCheckBox->SetTooltip("Toggle vertical sync");
		vsyncCheckBox->SetScriptTip("SetVSyncEnabled(opt bool enabled)");
		vsyncCheckBox->SetPos(XMFLOAT2(x, y += step));
		vsyncCheckBox->OnClick([](asEventArgs args) {
			asRenderer::GetDevice()->SetVSyncEnabled(args.bValue);
			});
		vsyncCheckBox->SetCheck(asRenderer::GetDevice()->GetVSyncEnabled());
		rendererWindow->AddWidget(vsyncCheckBox);

		occlusionCullingCheckBox = new asCheckBox("Occlusion Culling: ");
		occlusionCullingCheckBox->SetTooltip("Toggle occlusion culling. This can boost framerate if many objects are occluded in the scene.");
		occlusionCullingCheckBox->SetScriptTip("SetOcclusionCullingEnabled(bool enabled)");
		occlusionCullingCheckBox->SetPos(XMFLOAT2(x, y += step));
		occlusionCullingCheckBox->OnClick([](asEventArgs args) {
			asRenderer::SetOcclusionCullingEnabled(args.bValue);
			});
		occlusionCullingCheckBox->SetCheck(asRenderer::GetOcclusionCullingEnabled());
		rendererWindow->AddWidget(occlusionCullingCheckBox);

		resolutionScaleSlider = new asSlider(0.25f, 2.0f, 1.0f, 7.0f, "Resolution Scale: ");
		resolutionScaleSlider->SetTooltip("Adjust the internal rendering resolution.");
		resolutionScaleSlider->SetSize(XMFLOAT2(100, 30));
		resolutionScaleSlider->SetPos(XMFLOAT2(x, y += 30));
		resolutionScaleSlider->SetValue(asRenderer::GetResolutionScale());
		resolutionScaleSlider->OnSlide([&](asEventArgs args) {
			asRenderer::SetResolutionScale(args.fValue);
			});
		rendererWindow->AddWidget(resolutionScaleSlider);

		gammaSlider = new asSlider(1.0f, 3.0f, 2.2f, 1000.0f, "Gamma: ");
		gammaSlider->SetTooltip("Adjust the gamma correction for the display device.");
		gammaSlider->SetSize(XMFLOAT2(100, 30));
		gammaSlider->SetPos(XMFLOAT2(x, y += 30));
		gammaSlider->SetValue(asRenderer::GetGamma());
		gammaSlider->OnSlide([&](asEventArgs args) {
			asRenderer::SetGamma(args.fValue);
			});
		rendererWindow->AddWidget(gammaSlider);

		voxelRadianceCheckBox = new asCheckBox("Voxel GI: ");
		voxelRadianceCheckBox->SetTooltip("Toggle voxel Global Illumination computation.");
		voxelRadianceCheckBox->SetPos(XMFLOAT2(x, y += step));
		voxelRadianceCheckBox->OnClick([](asEventArgs args) {
			asRenderer::SetVoxelRadianceEnabled(args.bValue);
			});
		voxelRadianceCheckBox->SetCheck(asRenderer::GetVoxelRadianceEnabled());
		rendererWindow->AddWidget(voxelRadianceCheckBox);

		voxelRadianceDebugCheckBox = new asCheckBox("DEBUG: ");
		voxelRadianceDebugCheckBox->SetTooltip("Toggle Voxel GI visualization.");
		voxelRadianceDebugCheckBox->SetPos(XMFLOAT2(x + 130, y));
		voxelRadianceDebugCheckBox->OnClick([](asEventArgs args) {
			asRenderer::SetToDrawVoxelHelper(args.bValue);
			});
		voxelRadianceDebugCheckBox->SetCheck(asRenderer::GetToDrawVoxelHelper());
		rendererWindow->AddWidget(voxelRadianceDebugCheckBox);

		voxelRadianceSecondaryBounceCheckBox = new asCheckBox("Secondary Light Bounce: ");
		voxelRadianceSecondaryBounceCheckBox->SetTooltip("Toggle secondary light bounce computation for Voxel GI.");
		voxelRadianceSecondaryBounceCheckBox->SetPos(XMFLOAT2(x, y += step));
		voxelRadianceSecondaryBounceCheckBox->OnClick([](asEventArgs args) {
			asRenderer::SetVoxelRadianceSecondaryBounceEnabled(args.bValue);
			});
		voxelRadianceSecondaryBounceCheckBox->SetCheck(asRenderer::GetVoxelRadianceSecondaryBounceEnabled());
		rendererWindow->AddWidget(voxelRadianceSecondaryBounceCheckBox);

		voxelRadianceReflectionsCheckBox = new asCheckBox("Reflections: ");
		voxelRadianceReflectionsCheckBox->SetTooltip("Toggle specular reflections computation for Voxel GI.");
		voxelRadianceReflectionsCheckBox->SetPos(XMFLOAT2(x + 130, y));
		voxelRadianceReflectionsCheckBox->OnClick([](asEventArgs args) {
			asRenderer::SetVoxelRadianceReflectionsEnabled(args.bValue);
			});
		voxelRadianceReflectionsCheckBox->SetCheck(asRenderer::GetVoxelRadianceReflectionsEnabled());
		rendererWindow->AddWidget(voxelRadianceReflectionsCheckBox);

		voxelRadianceVoxelSizeSlider = new asSlider(0.25, 2, 1, 7, "Voxel GI Voxel Size: ");
		voxelRadianceVoxelSizeSlider->SetTooltip("Adjust the voxel size for Voxel GI calculations.");
		voxelRadianceVoxelSizeSlider->SetSize(XMFLOAT2(100, 30));
		voxelRadianceVoxelSizeSlider->SetPos(XMFLOAT2(x, y += 30));
		voxelRadianceVoxelSizeSlider->SetValue(asRenderer::GetVoxelRadianceVoxelSize());
		voxelRadianceVoxelSizeSlider->OnSlide([&](asEventArgs args) {
			asRenderer::SetVoxelRadianceVoxelSize(args.fValue);
			});
		rendererWindow->AddWidget(voxelRadianceVoxelSizeSlider);

		voxelRadianceConeTracingSlider = new asSlider(1, 16, 8, 15, "Voxel GI NumCones: ");
		voxelRadianceConeTracingSlider->SetTooltip("Adjust the number of cones sampled in the radiance gathering phase.");
		voxelRadianceConeTracingSlider->SetSize(XMFLOAT2(100, 30));
		voxelRadianceConeTracingSlider->SetPos(XMFLOAT2(x, y += 30));
		voxelRadianceConeTracingSlider->SetValue((float)asRenderer::GetVoxelRadianceNumCones());
		voxelRadianceConeTracingSlider->OnSlide([&](asEventArgs args) {
			asRenderer::SetVoxelRadianceNumCones(args.iValue);
			});
		rendererWindow->AddWidget(voxelRadianceConeTracingSlider);

		voxelRadianceRayStepSizeSlider = new asSlider(0.5f, 2.0f, 0.5f, 10000, "Voxel GI Ray Step Size: ");
		voxelRadianceRayStepSizeSlider->SetTooltip("Adjust the precision of ray marching for cone tracing step. Lower values = more precision but slower performance.");
		voxelRadianceRayStepSizeSlider->SetSize(XMFLOAT2(100, 30));
		voxelRadianceRayStepSizeSlider->SetPos(XMFLOAT2(x, y += 30));
		voxelRadianceRayStepSizeSlider->SetValue(asRenderer::GetVoxelRadianceRayStepSize());
		voxelRadianceRayStepSizeSlider->OnSlide([&](asEventArgs args) {
			asRenderer::SetVoxelRadianceRayStepSize(args.fValue);
			});
		rendererWindow->AddWidget(voxelRadianceRayStepSizeSlider);

		voxelRadianceMaxDistanceSlider = new asSlider(0, 100, 10, 10000, "Voxel GI Max Distance: ");
		voxelRadianceMaxDistanceSlider->SetTooltip("Adjust max raymarching distance for voxel GI.");
		voxelRadianceMaxDistanceSlider->SetSize(XMFLOAT2(100, 30));
		voxelRadianceMaxDistanceSlider->SetPos(XMFLOAT2(x, y += 30));
		voxelRadianceMaxDistanceSlider->SetValue(asRenderer::GetVoxelRadianceMaxDistance());
		voxelRadianceMaxDistanceSlider->OnSlide([&](asEventArgs args) {
			asRenderer::SetVoxelRadianceMaxDistance(args.fValue);
			});
		rendererWindow->AddWidget(voxelRadianceMaxDistanceSlider);

		wireFrameCheckBox = new asCheckBox("Render Wireframe: ");
		wireFrameCheckBox->SetTooltip("Visualize the scene as a wireframe");
		wireFrameCheckBox->SetPos(XMFLOAT2(x, y += step));
		wireFrameCheckBox->OnClick([](asEventArgs args) {
			asRenderer::SetWireRender(args.bValue);
			});
		wireFrameCheckBox->SetCheck(asRenderer::IsWireRender());
		rendererWindow->AddWidget(wireFrameCheckBox);

		advancedLightCullingCheckBox = new asCheckBox("2.5D Light Culling: ");
		advancedLightCullingCheckBox->SetTooltip("Enable a more aggressive light culling approach which can result in slower culling but faster rendering (Tiled renderer only)");
		advancedLightCullingCheckBox->SetPos(XMFLOAT2(x, y += step));
		advancedLightCullingCheckBox->OnClick([](asEventArgs args) {
			asRenderer::SetAdvancedLightCulling(args.bValue);
			});
		advancedLightCullingCheckBox->SetCheck(asRenderer::GetAdvancedLightCulling());
		rendererWindow->AddWidget(advancedLightCullingCheckBox);

		debugLightCullingCheckBox = new asCheckBox("DEBUG: ");
		debugLightCullingCheckBox->SetTooltip("Toggle visualization of the screen space light culling heatmap grid (Tiled renderer only)");
		debugLightCullingCheckBox->SetPos(XMFLOAT2(x + 100, y));
		debugLightCullingCheckBox->OnClick([](asEventArgs args) {
			asRenderer::SetDebugLightCulling(args.bValue);
			});
		debugLightCullingCheckBox->SetCheck(asRenderer::GetDebugLightCulling());
		rendererWindow->AddWidget(debugLightCullingCheckBox);

		tessellationCheckBox = new asCheckBox("Tessellation Enabled: ");
		tessellationCheckBox->SetTooltip("Enable tessellation feature. You also need to specify a tessellation factor for individual objects.");
		tessellationCheckBox->SetPos(XMFLOAT2(x, y += step));
		tessellationCheckBox->OnClick([=](asEventArgs args) {
			path->setTessellationEnabled(args.bValue);
			});
		tessellationCheckBox->SetCheck(false);
		rendererWindow->AddWidget(tessellationCheckBox);
		tessellationCheckBox->SetEnabled(asRenderer::GetDevice()->CheckCapability(asGraphics::GraphicsDevice::GRAPHICSDEVICE_CAPABILITY_TESSELLATION));

		advancedRefractionsCheckBox = new asCheckBox("Advanced Refractions: ");
		advancedRefractionsCheckBox->SetTooltip("Enable advanced refraction rendering: rough transparent materials will be more matte.");
		advancedRefractionsCheckBox->SetPos(XMFLOAT2(x, y += step));
		advancedRefractionsCheckBox->OnClick([=](asEventArgs args) {
			asRenderer::SetAdvancedRefractionsEnabled(args.bValue);
			});
		advancedRefractionsCheckBox->SetCheck(asRenderer::GetAdvancedRefractionsEnabled());
		rendererWindow->AddWidget(advancedRefractionsCheckBox);

		alphaCompositionCheckBox = new asCheckBox("Alpha Composition: ");
		alphaCompositionCheckBox->SetTooltip("Enable Alpha Composition. Enables softer alpha blending on partly solid geometry (eg. vegetation) but rendering performance will be slower.");
		alphaCompositionCheckBox->SetPos(XMFLOAT2(x, y += step));
		alphaCompositionCheckBox->OnClick([=](asEventArgs args) {
			asRenderer::SetAlphaCompositionEnabled(args.bValue);
			});
		alphaCompositionCheckBox->SetCheck(asRenderer::GetAlphaCompositionEnabled());
		rendererWindow->AddWidget(alphaCompositionCheckBox);

		speedMultiplierSlider = new asSlider(0, 4, 1, 100000, "Speed: ");
		speedMultiplierSlider->SetTooltip("Adjust the global speed (time multiplier)");
		speedMultiplierSlider->SetSize(XMFLOAT2(100, 30));
		speedMultiplierSlider->SetPos(XMFLOAT2(x, y += 30));
		speedMultiplierSlider->SetValue(asRenderer::GetGameSpeed());
		speedMultiplierSlider->OnSlide([&](asEventArgs args) {
			asRenderer::SetGameSpeed(args.fValue);
			});
		rendererWindow->AddWidget(speedMultiplierSlider);

		transparentShadowsCheckBox = new asCheckBox("Transparent Shadows: ");
		transparentShadowsCheckBox->SetTooltip("Enables color tinted shadows and refraction caustics effects for transparent objects and water.");
		transparentShadowsCheckBox->SetPos(XMFLOAT2(x, y += step));
		transparentShadowsCheckBox->SetCheck(asRenderer::GetTransparentShadowsEnabled());
		transparentShadowsCheckBox->OnClick([=](asEventArgs args) {
			asRenderer::SetTransparentShadowsEnabled(args.bValue);
			});
		rendererWindow->AddWidget(transparentShadowsCheckBox);

		shadowProps2DComboBox = new asComboBox("2D Shadowmap resolution:");
		shadowProps2DComboBox->SetSize(XMFLOAT2(100, 20));
		shadowProps2DComboBox->SetPos(XMFLOAT2(x, y += step));
		shadowProps2DComboBox->AddItem("Off");
		shadowProps2DComboBox->AddItem("128");
		shadowProps2DComboBox->AddItem("256");
		shadowProps2DComboBox->AddItem("512");
		shadowProps2DComboBox->AddItem("1024");
		shadowProps2DComboBox->AddItem("2048");
		shadowProps2DComboBox->AddItem("4096");
		shadowProps2DComboBox->OnSelect([&](asEventArgs args) {

			switch (args.iValue)
			{
			case 0:
				asRenderer::SetShadowProps2D(0, -1, -1);
				break;
			case 1:
				asRenderer::SetShadowProps2D(128, -1, -1);
				break;
			case 2:
				asRenderer::SetShadowProps2D(256, -1, -1);
				break;
			case 3:
				asRenderer::SetShadowProps2D(512, -1, -1);
				break;
			case 4:
				asRenderer::SetShadowProps2D(1024, -1, -1);
				break;
			case 5:
				asRenderer::SetShadowProps2D(2048, -1, -1);
				break;
			case 6:
				asRenderer::SetShadowProps2D(4096, -1, -1);
				break;
			default:
				break;
			}
			});
		shadowProps2DComboBox->SetSelected(4);
		shadowProps2DComboBox->SetEnabled(true);
		shadowProps2DComboBox->SetTooltip("Choose a shadow quality preset for 2D shadow maps (spotlights, directional lights)...");
		shadowProps2DComboBox->SetScriptTip("SetShadowProps2D(int resolution, int count, int softShadowQuality)");
		rendererWindow->AddWidget(shadowProps2DComboBox);

		shadowPropsCubeComboBox = new asComboBox("Cube Shadowmap resolution:");
		shadowPropsCubeComboBox->SetSize(XMFLOAT2(100, 20));
		shadowPropsCubeComboBox->SetPos(XMFLOAT2(x, y += step));
		shadowPropsCubeComboBox->AddItem("Off");
		shadowPropsCubeComboBox->AddItem("128");
		shadowPropsCubeComboBox->AddItem("256");
		shadowPropsCubeComboBox->AddItem("512");
		shadowPropsCubeComboBox->AddItem("1024");
		shadowPropsCubeComboBox->AddItem("2048");
		shadowPropsCubeComboBox->AddItem("4096");
		shadowPropsCubeComboBox->OnSelect([&](asEventArgs args) {
			switch (args.iValue)
			{
			case 0:
				asRenderer::SetShadowPropsCube(0, -1);
				break;
			case 1:
				asRenderer::SetShadowPropsCube(128, -1);
				break;
			case 2:
				asRenderer::SetShadowPropsCube(256, -1);
				break;
			case 3:
				asRenderer::SetShadowPropsCube(512, -1);
				break;
			case 4:
				asRenderer::SetShadowPropsCube(1024, -1);
				break;
			case 5:
				asRenderer::SetShadowPropsCube(2048, -1);
				break;
			case 6:
				asRenderer::SetShadowPropsCube(4096, -1);
				break;
			default:
				break;
			}
			});
		shadowPropsCubeComboBox->SetSelected(2);
		shadowPropsCubeComboBox->SetEnabled(true);
		shadowPropsCubeComboBox->SetTooltip("Choose a shadow quality preset for cube shadow maps (pointlights, area lights)...");
		shadowPropsCubeComboBox->SetScriptTip("SetShadowPropsCube(int resolution, int count)");
		rendererWindow->AddWidget(shadowPropsCubeComboBox);

		MSAAComboBox = new asComboBox("MSAA:");
		MSAAComboBox->SetSize(XMFLOAT2(100, 20));
		MSAAComboBox->SetPos(XMFLOAT2(x, y += step));
		MSAAComboBox->AddItem("Off");
		MSAAComboBox->AddItem("2");
		MSAAComboBox->AddItem("4");
		MSAAComboBox->AddItem("8");
		MSAAComboBox->OnSelect([=](asEventArgs args) {
			switch (args.iValue)
			{
			case 0:
				path->setMSAASampleCount(1);
				break;
			case 1:
				path->setMSAASampleCount(2);
				break;
			case 2:
				path->setMSAASampleCount(4);
				break;
			case 3:
				path->setMSAASampleCount(8);
				break;
			default:
				break;
			}
			editorcomponent->ResizeBuffers();
			});
		MSAAComboBox->SetSelected(0);
		MSAAComboBox->SetEnabled(true);
		MSAAComboBox->SetTooltip("Multisampling Anti Aliasing quality. It is only available for Forward render paths.");
		rendererWindow->AddWidget(MSAAComboBox);

		temporalAACheckBox = new asCheckBox("Temporal AA: ");
		temporalAACheckBox->SetTooltip("Toggle Temporal Anti Aliasing. It is a supersampling techique which is performed across multiple frames.");
		temporalAACheckBox->SetPos(XMFLOAT2(x, y += step));
		temporalAACheckBox->OnClick([](asEventArgs args) {
			asRenderer::SetTemporalAAEnabled(args.bValue);
			});
		temporalAACheckBox->SetCheck(asRenderer::GetTemporalAAEnabled());
		rendererWindow->AddWidget(temporalAACheckBox);

		temporalAADebugCheckBox = new asCheckBox("DEBUGJitter: ");
		temporalAADebugCheckBox->SetText("DEBUG: ");
		temporalAADebugCheckBox->SetTooltip("Disable blending of frame history. Camera Subpixel jitter will be visible.");
		temporalAADebugCheckBox->SetPos(XMFLOAT2(x + 100, y));
		temporalAADebugCheckBox->OnClick([](asEventArgs args) {
			asRenderer::SetTemporalAADebugEnabled(args.bValue);
			});
		temporalAADebugCheckBox->SetCheck(asRenderer::GetTemporalAADebugEnabled());
		rendererWindow->AddWidget(temporalAADebugCheckBox);

		textureQualityComboBox = new asComboBox("Texture Quality:");
		textureQualityComboBox->SetSize(XMFLOAT2(100, 20));
		textureQualityComboBox->SetPos(XMFLOAT2(x, y += step));
		textureQualityComboBox->AddItem("Nearest");
		textureQualityComboBox->AddItem("Bilinear");
		textureQualityComboBox->AddItem("Trilinear");
		textureQualityComboBox->AddItem("Anisotropic");
		textureQualityComboBox->OnSelect([&](asEventArgs args) {
			asGraphics::SamplerDesc desc = asRenderer::GetSampler(SSLOT_OBJECTSHADER)->GetDesc();

			switch (args.iValue)
			{
			case 0:
				desc.Filter = asGraphics::FILTER_MIN_MAG_MIP_POINT;
				break;
			case 1:
				desc.Filter = asGraphics::FILTER_MIN_MAG_LINEAR_MIP_POINT;
				break;
			case 2:
				desc.Filter = asGraphics::FILTER_MIN_MAG_MIP_LINEAR;
				break;
			case 3:
				desc.Filter = asGraphics::FILTER_ANISOTROPIC;
				break;
			default:
				break;
			}

			asRenderer::ModifySampler(desc, SSLOT_OBJECTSHADER);

			});
		textureQualityComboBox->SetSelected(3);
		textureQualityComboBox->SetEnabled(true);
		textureQualityComboBox->SetTooltip("Choose a texture sampling method for material textures.");
		rendererWindow->AddWidget(textureQualityComboBox);

		mipLodBiasSlider = new asSlider(-2, 2, 0, 100000, "MipLOD Bias: ");
		mipLodBiasSlider->SetTooltip("Bias the rendered mip map level of the material textures.");
		mipLodBiasSlider->SetSize(XMFLOAT2(100, 30));
		mipLodBiasSlider->SetPos(XMFLOAT2(x, y += 30));
		mipLodBiasSlider->OnSlide([&](asEventArgs args) {
			asGraphics::SamplerDesc desc = asRenderer::GetSampler(SSLOT_OBJECTSHADER)->GetDesc();
			desc.MipLODBias = args.fValue;
			asRenderer::ModifySampler(desc, SSLOT_OBJECTSHADER);
			});
		rendererWindow->AddWidget(mipLodBiasSlider);

		raytraceBounceCountSlider = new asSlider(0, 10, 1, 10, "Raytrace Bounces: ");
		raytraceBounceCountSlider->SetTooltip("How many indirect light bounces to compute when doing ray tracing.");
		raytraceBounceCountSlider->SetSize(XMFLOAT2(100, 30));
		raytraceBounceCountSlider->SetPos(XMFLOAT2(x, y += 30));
		raytraceBounceCountSlider->SetValue((float)asRenderer::GetRaytraceBounceCount());
		raytraceBounceCountSlider->OnSlide([&](asEventArgs args) {
			asRenderer::SetRaytraceBounceCount((uint32_t)args.iValue);
			});
		rendererWindow->AddWidget(raytraceBounceCountSlider);



		// Visualizer toggles:
		x = 600, y = 20;

		partitionBoxesCheckBox = new asCheckBox("SPTree visualizer: ");
		partitionBoxesCheckBox->SetTooltip("Visualize the world space partitioning tree as boxes");
		partitionBoxesCheckBox->SetScriptTip("SetDebugPartitionTreeEnabled(bool enabled)");
		partitionBoxesCheckBox->SetPos(XMFLOAT2(x, y += step));
		partitionBoxesCheckBox->OnClick([](asEventArgs args) {
			asRenderer::SetToDrawDebugPartitionTree(args.bValue);
			});
		partitionBoxesCheckBox->SetCheck(asRenderer::GetToDrawDebugPartitionTree());
		partitionBoxesCheckBox->SetEnabled(false); // SP tree is not implemented at the moment anymore
		rendererWindow->AddWidget(partitionBoxesCheckBox);

		boneLinesCheckBox = new asCheckBox("Bone line visualizer: ");
		boneLinesCheckBox->SetTooltip("Visualize bones of armatures");
		boneLinesCheckBox->SetScriptTip("SetDebugBonesEnabled(bool enabled)");
		boneLinesCheckBox->SetPos(XMFLOAT2(x, y += step));
		boneLinesCheckBox->OnClick([](asEventArgs args) {
			asRenderer::SetToDrawDebugBoneLines(args.bValue);
			});
		boneLinesCheckBox->SetCheck(asRenderer::GetToDrawDebugBoneLines());
		rendererWindow->AddWidget(boneLinesCheckBox);

		debugEmittersCheckBox = new asCheckBox("Emitter visualizer: ");
		debugEmittersCheckBox->SetTooltip("Visualize emitters");
		debugEmittersCheckBox->SetScriptTip("SetDebugEmittersEnabled(bool enabled)");
		debugEmittersCheckBox->SetPos(XMFLOAT2(x, y += step));
		debugEmittersCheckBox->OnClick([](asEventArgs args) {
			asRenderer::SetToDrawDebugEmitters(args.bValue);
			});
		debugEmittersCheckBox->SetCheck(asRenderer::GetToDrawDebugEmitters());
		rendererWindow->AddWidget(debugEmittersCheckBox);

		debugForceFieldsCheckBox = new asCheckBox("Force Field visualizer: ");
		debugForceFieldsCheckBox->SetTooltip("Visualize force fields");
		debugForceFieldsCheckBox->SetScriptTip("SetDebugForceFieldsEnabled(bool enabled)");
		debugForceFieldsCheckBox->SetPos(XMFLOAT2(x, y += step));
		debugForceFieldsCheckBox->OnClick([](asEventArgs args) {
			asRenderer::SetToDrawDebugForceFields(args.bValue);
			});
		debugForceFieldsCheckBox->SetCheck(asRenderer::GetToDrawDebugForceFields());
		rendererWindow->AddWidget(debugForceFieldsCheckBox);

		debugRaytraceBVHCheckBox = new asCheckBox("Raytrace BVH visualizer: ");
		debugRaytraceBVHCheckBox->SetTooltip("Visualize scene BVH if raytracing is enabled");
		debugRaytraceBVHCheckBox->SetPos(XMFLOAT2(x, y += step));
		debugRaytraceBVHCheckBox->OnClick([](asEventArgs args) {
			asRenderer::SetRaytraceDebugBVHVisualizerEnabled(args.bValue);
			});
		debugRaytraceBVHCheckBox->SetCheck(asRenderer::GetRaytraceDebugBVHVisualizerEnabled());
		rendererWindow->AddWidget(debugRaytraceBVHCheckBox);

		envProbesCheckBox = new asCheckBox("Env probe visualizer: ");
		envProbesCheckBox->SetTooltip("Toggle visualization of environment probes as reflective spheres");
		envProbesCheckBox->SetPos(XMFLOAT2(x, y += step));
		envProbesCheckBox->OnClick([](asEventArgs args) {
			asRenderer::SetToDrawDebugEnvProbes(args.bValue);
			});
		envProbesCheckBox->SetCheck(asRenderer::GetToDrawDebugEnvProbes());
		rendererWindow->AddWidget(envProbesCheckBox);

		cameraVisCheckBox = new asCheckBox("Camera Proxy visualizer: ");
		cameraVisCheckBox->SetTooltip("Toggle visualization of camera proxies in the scene");
		cameraVisCheckBox->SetPos(XMFLOAT2(x, y += step));
		cameraVisCheckBox->OnClick([](asEventArgs args) {
			asRenderer::SetToDrawDebugCameras(args.bValue);
			});
		cameraVisCheckBox->SetCheck(asRenderer::GetToDrawDebugCameras());
		rendererWindow->AddWidget(cameraVisCheckBox);

		gridHelperCheckBox = new asCheckBox("Grid helper: ");
		gridHelperCheckBox->SetTooltip("Toggle showing of unit visualizer grid in the world origin");
		gridHelperCheckBox->SetPos(XMFLOAT2(x, y += step));
		gridHelperCheckBox->OnClick([](asEventArgs args) {
			asRenderer::SetToDrawGridHelper(args.bValue);
			});
		gridHelperCheckBox->SetCheck(asRenderer::GetToDrawGridHelper());
		rendererWindow->AddWidget(gridHelperCheckBox);


		pickTypeObjectCheckBox = new asCheckBox("Pick Objects: ");
		pickTypeObjectCheckBox->SetTooltip("Enable if you want to pick objects with the pointer");
		pickTypeObjectCheckBox->SetPos(XMFLOAT2(x, y += step * 2));
		pickTypeObjectCheckBox->SetCheck(true);
		rendererWindow->AddWidget(pickTypeObjectCheckBox);

		pickTypeEnvProbeCheckBox = new asCheckBox("Pick EnvProbes: ");
		pickTypeEnvProbeCheckBox->SetTooltip("Enable if you want to pick environment probes with the pointer");
		pickTypeEnvProbeCheckBox->SetPos(XMFLOAT2(x, y += step));
		pickTypeEnvProbeCheckBox->SetCheck(true);
		rendererWindow->AddWidget(pickTypeEnvProbeCheckBox);

		pickTypeLightCheckBox = new asCheckBox("Pick Lights: ");
		pickTypeLightCheckBox->SetTooltip("Enable if you want to pick lights with the pointer");
		pickTypeLightCheckBox->SetPos(XMFLOAT2(x, y += step));
		pickTypeLightCheckBox->SetCheck(true);
		rendererWindow->AddWidget(pickTypeLightCheckBox);

		pickTypeDecalCheckBox = new asCheckBox("Pick Decals: ");
		pickTypeDecalCheckBox->SetTooltip("Enable if you want to pick decals with the pointer");
		pickTypeDecalCheckBox->SetPos(XMFLOAT2(x, y += step));
		pickTypeDecalCheckBox->SetCheck(true);
		rendererWindow->AddWidget(pickTypeDecalCheckBox);

		pickTypeForceFieldCheckBox = new asCheckBox("Pick Force Fields: ");
		pickTypeForceFieldCheckBox->SetTooltip("Enable if you want to pick force fields with the pointer");
		pickTypeForceFieldCheckBox->SetPos(XMFLOAT2(x, y += step));
		pickTypeForceFieldCheckBox->SetCheck(true);
		rendererWindow->AddWidget(pickTypeForceFieldCheckBox);

		pickTypeEmitterCheckBox = new asCheckBox("Pick Emitters: ");
		pickTypeEmitterCheckBox->SetTooltip("Enable if you want to pick emitters with the pointer");
		pickTypeEmitterCheckBox->SetPos(XMFLOAT2(x, y += step));
		pickTypeEmitterCheckBox->SetCheck(true);
		rendererWindow->AddWidget(pickTypeEmitterCheckBox);

		pickTypeHairCheckBox = new asCheckBox("Pick Hairs: ");
		pickTypeHairCheckBox->SetTooltip("Enable if you want to pick hairs with the pointer");
		pickTypeHairCheckBox->SetPos(XMFLOAT2(x, y += step));
		pickTypeHairCheckBox->SetCheck(true);
		rendererWindow->AddWidget(pickTypeHairCheckBox);

		pickTypeCameraCheckBox = new asCheckBox("Pick Cameras: ");
		pickTypeCameraCheckBox->SetTooltip("Enable if you want to pick cameras with the pointer");
		pickTypeCameraCheckBox->SetPos(XMFLOAT2(x, y += step));
		pickTypeCameraCheckBox->SetCheck(true);
		rendererWindow->AddWidget(pickTypeCameraCheckBox);

		pickTypeArmatureCheckBox = new asCheckBox("Pick Armatures: ");
		pickTypeArmatureCheckBox->SetTooltip("Enable if you want to pick armatures with the pointer");
		pickTypeArmatureCheckBox->SetPos(XMFLOAT2(x, y += step));
		pickTypeArmatureCheckBox->SetCheck(true);
		rendererWindow->AddWidget(pickTypeArmatureCheckBox);

		pickTypeSoundCheckBox = new asCheckBox("Pick Sounds: ");
		pickTypeSoundCheckBox->SetTooltip("Enable if you want to pick sounds with the pointer");
		pickTypeSoundCheckBox->SetPos(XMFLOAT2(x, y += step));
		pickTypeSoundCheckBox->SetCheck(true);
		rendererWindow->AddWidget(pickTypeSoundCheckBox);



		freezeCullingCameraCheckBox = new asCheckBox("Freeze culling camera: ");
		freezeCullingCameraCheckBox->SetTooltip("Freeze culling camera update. Scene culling will not be updated with the view");
		freezeCullingCameraCheckBox->SetPos(XMFLOAT2(x, y += step * 2));
		freezeCullingCameraCheckBox->OnClick([](asEventArgs args) {
			asRenderer::SetFreezeCullingCameraEnabled(args.bValue);
			});
		freezeCullingCameraCheckBox->SetCheck(asRenderer::GetToDrawDebugForceFields());
		rendererWindow->AddWidget(freezeCullingCameraCheckBox);



		rendererWindow->Translate(XMFLOAT3(130, 20, 0));
		rendererWindow->SetVisible(false);
	}


	RendererWindow::~RendererWindow()
	{
		rendererWindow->RemoveWidgets(true);
		GUI->RemoveWidget(rendererWindow);
		SAFE_DELETE(rendererWindow);
	}

	UINT RendererWindow::GetPickType()
	{
		UINT pickType = PICK_VOID;
		if (pickTypeObjectCheckBox->GetCheck())
		{
			pickType |= PICK_OBJECT;
		}
		if (pickTypeEnvProbeCheckBox->GetCheck())
		{
			pickType |= PICK_ENVPROBE;
		}
		if (pickTypeLightCheckBox->GetCheck())
		{
			pickType |= PICK_LIGHT;
		}
		if (pickTypeDecalCheckBox->GetCheck())
		{
			pickType |= PICK_DECAL;
		}
		if (pickTypeForceFieldCheckBox->GetCheck())
		{
			pickType |= PICK_FORCEFIELD;
		}
		if (pickTypeEmitterCheckBox->GetCheck())
		{
			pickType |= PICK_EMITTER;
		}
		if (pickTypeHairCheckBox->GetCheck())
		{
			pickType |= PICK_HAIR;
		}
		if (pickTypeCameraCheckBox->GetCheck())
		{
			pickType |= PICK_CAMERA;
		}
		if (pickTypeArmatureCheckBox->GetCheck())
		{
			pickType |= PICK_ARMATURE;
		}
		if (pickTypeSoundCheckBox->GetCheck())
		{
			pickType |= PICK_SOUND;
		}

		return pickType;
	}
}