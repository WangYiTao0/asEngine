#pragma once

namespace as
{

	class EditorComponent;

	class asGUI;
	class asWindow;
	class asLabel;
	class asCheckBox;
	class asSlider;
	class asComboBox;

	enum PICKTYPE
	{
		PICK_VOID = 0,
		PICK_OBJECT = RENDERTYPE_OPAQUE | RENDERTYPE_TRANSPARENT | RENDERTYPE_WATER,
		PICK_LIGHT = 8,
		PICK_DECAL = 16,
		PICK_ENVPROBE = 32,
		PICK_FORCEFIELD = 64,
		PICK_EMITTER = 128,
		PICK_HAIR = 256,
		PICK_CAMERA = 512,
		PICK_ARMATURE = 1024,
		PICK_SOUND = 2048,
	};

	class RendererWindow
	{
	public:
		RendererWindow(asGUI* gui, EditorComponent* editorcomponent, RenderPath3D* path);
		~RendererWindow();

		asGUI* GUI;

		asWindow* rendererWindow;
		asCheckBox* vsyncCheckBox;
		asCheckBox* occlusionCullingCheckBox;
		asSlider* resolutionScaleSlider;
		asSlider* gammaSlider;
		asCheckBox* voxelRadianceCheckBox;
		asCheckBox* voxelRadianceDebugCheckBox;
		asCheckBox* voxelRadianceSecondaryBounceCheckBox;
		asCheckBox* voxelRadianceReflectionsCheckBox;
		asSlider* voxelRadianceVoxelSizeSlider;
		asSlider* voxelRadianceConeTracingSlider;
		asSlider* voxelRadianceRayStepSizeSlider;
		asSlider* voxelRadianceMaxDistanceSlider;
		asCheckBox* partitionBoxesCheckBox;
		asCheckBox* boneLinesCheckBox;
		asCheckBox* debugEmittersCheckBox;
		asCheckBox* debugForceFieldsCheckBox;
		asCheckBox* debugRaytraceBVHCheckBox;
		asCheckBox* wireFrameCheckBox;
		asCheckBox* advancedLightCullingCheckBox;
		asCheckBox* debugLightCullingCheckBox;
		asCheckBox* tessellationCheckBox;
		asCheckBox* advancedRefractionsCheckBox;
		asCheckBox* alphaCompositionCheckBox;
		asCheckBox* envProbesCheckBox;
		asCheckBox* gridHelperCheckBox;
		asCheckBox* cameraVisCheckBox;
		asCheckBox* pickTypeObjectCheckBox;
		asCheckBox* pickTypeEnvProbeCheckBox;
		asCheckBox* pickTypeLightCheckBox;
		asCheckBox* pickTypeDecalCheckBox;
		asCheckBox* pickTypeForceFieldCheckBox;
		asCheckBox* pickTypeEmitterCheckBox;
		asCheckBox* pickTypeHairCheckBox;
		asCheckBox* pickTypeCameraCheckBox;
		asCheckBox* pickTypeArmatureCheckBox;
		asCheckBox* pickTypeSoundCheckBox;
		asSlider* speedMultiplierSlider;
		asCheckBox* transparentShadowsCheckBox;
		asComboBox* shadowProps2DComboBox;
		asComboBox* shadowPropsCubeComboBox;
		asComboBox* MSAAComboBox;
		asCheckBox* temporalAACheckBox;
		asCheckBox* temporalAADebugCheckBox;
		asComboBox* textureQualityComboBox;
		asSlider* mipLodBiasSlider;
		asSlider* raytraceBounceCountSlider;

		asCheckBox* freezeCullingCameraCheckBox;

		UINT GetPickType();
	};
}
