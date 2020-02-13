#pragma once
namespace as
{
	class asGUI;
	class asWindow;
	class asLabel;
	class asCheckBox;
	class asSlider;
	class asColorPicker;
	class asButton;
	class asComboBox;
	class asTextInputField;

	class MaterialWindow
	{
	public:
		MaterialWindow(asGUI* gui);
		~MaterialWindow();

		asECS::Entity entity;
		void SetEntity(asECS::Entity entity);

		asGUI* GUI;

		asWindow* materialWindow;
		asTextInputField* materialNameField;
		asCheckBox* waterCheckBox;
		asCheckBox* planarReflCheckBox;
		asCheckBox* shadowCasterCheckBox;
		asCheckBox* flipNormalMapCheckBox;
		asCheckBox* useVertexColorsCheckBox;
		asCheckBox* specularGlossinessCheckBox;
		asCheckBox* occlusionPrimaryCheckBox;
		asCheckBox* occlusionSecondaryCheckBox;
		asSlider* normalMapSlider;
		asSlider* roughnessSlider;
		asSlider* reflectanceSlider;
		asSlider* metalnessSlider;
		asSlider* alphaSlider;
		asSlider* refractionIndexSlider;
		asSlider* emissiveSlider;
		asSlider* sssSlider;
		asSlider* pomSlider;
		asSlider* displacementMappingSlider;
		asSlider* texAnimFrameRateSlider;
		asSlider* texAnimDirectionSliderU;
		asSlider* texAnimDirectionSliderV;
		asSlider* texMulSliderX;
		asSlider* texMulSliderY;
		asColorPicker* baseColorPicker;
		asColorPicker* emissiveColorPicker;
		asSlider* alphaRefSlider;
		asComboBox* blendModeComboBox;
		asComboBox* shaderTypeComboBox;

		asLabel* texture_baseColor_Label;
		asLabel* texture_normal_Label;
		asLabel* texture_surface_Label;
		asLabel* texture_displacement_Label;
		asLabel* texture_emissive_Label;
		asLabel* texture_occlusion_Label;

		asButton* texture_baseColor_Button;
		asButton* texture_normal_Button;
		asButton* texture_surface_Button;
		asButton* texture_displacement_Button;
		asButton* texture_emissive_Button;
		asButton* texture_occlusion_Button;

		asTextInputField* texture_baseColor_uvset_Field;
		asTextInputField* texture_normal_uvset_Field;
		asTextInputField* texture_surface_uvset_Field;
		asTextInputField* texture_displacement_uvset_Field;
		asTextInputField* texture_emissive_uvset_Field;
		asTextInputField* texture_occlusion_uvset_Field;
	};

}