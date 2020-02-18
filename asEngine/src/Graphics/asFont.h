#pragma once
#include "CommonInclude.h"
#include "Graphics/API/asGraphicsDevice.h"
#include "Helpers/asColors.h"
#include "Helpers\asHashString.h"

namespace as
{

	// Do not alter order because it is bound to lua manually
	enum asFontAlign
	{
		ASFALIGN_LEFT,
		ASFALIGN_CENTER,
		ASFALIGN_RIGHT,
		ASFALIGN_TOP,
		ASFALIGN_BOTTOM
	};

	static const int ASFONTSIZE_DEFAULT = 14;

	struct asFontParams
	{
		int posX, posY;
		int size = ASFONTSIZE_DEFAULT; // line height in pixels
		float scaling = 1;
		int spacingX, spacingY;
		asFontAlign h_align, v_align;
		asColor color;
		asColor shadowColor;

		asFontParams(int posX = 0, int posY = 0, int size = 16, asFontAlign h_align = ASFALIGN_LEFT, asFontAlign v_align = ASFALIGN_TOP
			, int spacingX = 0, int spacingY = 0, asColor color = asColor(255, 255, 255, 255), asColor shadowColor = asColor(0, 0, 0, 0))
			:posX(posX), posY(posY), size(size), h_align(h_align), v_align(v_align), spacingX(spacingX), spacingY(spacingY), color(color), shadowColor(shadowColor)
		{}
	};


	class asFont
	{
	public:
		static void Initialize();

		static void LoadShaders();
		static const as::asGraphics::Texture* GetAtlas();

		// Returns the font directory
		static const std::string& GetFontPath();
		// Sets the font directory
		static void SetFontPath(const std::string& path);

		// Create a font. Returns fontStyleID that is reusable. If font already exists, just return its ID
		static int AddFontStyle(const std::string& fontName);

		std::wstring text;
		asFontParams params;
		int style;

		asFont(const std::string& text = "", asFontParams params = asFontParams(), int style = 0);
		asFont(const std::wstring& text, asFontParams params = asFontParams(), int style = 0);

		void Draw(as::asGraphics::CommandList cmd) const;

		int textWidth() const;
		int textHeight() const;

		void SetText(const std::string& text);
		void SetText(const std::wstring& text);
		std::wstring GetText() const;
		std::string GetTextA() const;

	};
}
