#include "aspch.h"
#include "asFont.h"
#include "Graphics/asRenderer.h"
#include "Helpers/asResourceManager.h"
#include "Helpers/asHelper.h"
#include "Graphics/GPUMapping/ResourceMapping.h"
#include "Graphics/GPUMapping/ShaderInterop_Font.h"
#include "Tools/asBackLog.h"
#include "Graphics/asTextureHelper.h"
#include "Helpers/asRectPacker.h"
#include "Helpers/asSpinLock.h"

#include "Utility/stb_truetype.h"



using namespace std;
using namespace as::asGraphics;
using namespace as::asRectPacker;

#define MAX_TEXT 10000
#define WHITESPACE_SIZE (int((params.size + params.spacingX) * params.scaling * 0.3f))
#define TAB_SIZE (WHITESPACE_SIZE * 4)
#define LINEBREAK_SIZE (int((params.size + params.spacingY) * params.scaling))

namespace as
{
	namespace asFont_Internal
	{
		std::string			FONTPATH = asHelper::GetOriginalWorkingDirectory() + "../WickedEngine/fonts/";
		GPUBuffer			indexBuffer;
		GPUBuffer			constantBuffer;
		BlendState			blendState;
		RasterizerState		rasterizerState;
		DepthStencilState	depthStencilState;
		Sampler				sampler;

		VertexLayout		vertexLayout;
		VertexShader		vertexShader;
		PixelShader			pixelShader;
		PipelineState		PSO;

		atomic_bool initialized = false;

		Texture texture;

		struct Glyph
		{
			int16_t x;
			int16_t y;
			int16_t width;
			int16_t height;
			uint16_t tc_left;
			uint16_t tc_right;
			uint16_t tc_top;
			uint16_t tc_bottom;
		};
		unordered_map<int32_t, Glyph> glyph_lookup;
		unordered_map<int32_t, rect_xywh> rect_lookup;
		// pack glyph identifiers to a 32-bit hash:
		//	height:	10 bits	(height supported: 0 - 1023)
		//	style:	6 bits	(number of font styles supported: 0 - 63)
		//	code:	16 bits (character code range supported: 0 - 65535)
		constexpr int32_t glyphhash(int code, int style, int height) { return ((code & 0xFFFF) << 16) | ((style & 0x3F) << 10) | (height & 0x3FF); }
		constexpr int codefromhash(int64_t hash) { return int((hash >> 16) & 0xFFFF); }
		constexpr int stylefromhash(int64_t hash) { return int((hash >> 10) & 0x3F); }
		constexpr int heightfromhash(int64_t hash) { return int((hash >> 0) & 0x3FF); }
		unordered_set<int32_t> pendingGlyphs;
		asSpinLock glyphLock;

		struct asFontStyle
		{
			string name;
			vector<uint8_t> fontBuffer;
			stbtt_fontinfo fontInfo;
			int ascent, descent, lineGap;
			void Create(const string& newName)
			{
				name = newName;
				asHelper::readByteData(newName, fontBuffer);

				int offset = stbtt_GetFontOffsetForIndex(fontBuffer.data(), 0);

				if (!stbtt_InitFont(&fontInfo, fontBuffer.data(), offset))
				{
					stringstream ss("");
					ss << "Failed to load font: " << name;
					asHelper::messageBox(ss.str());
				}

				stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);
			}
		};
		std::vector<asFontStyle> fontStyles;

		struct FontVertex
		{
			XMSHORT2 Pos;
			XMHALF2 Tex;
		};

		uint32_t WriteVertices(volatile FontVertex* vertexList, const std::wstring& text, asFontParams params, int style)
		{
			uint32_t quadCount = 0;

			int16_t line = 0;
			int16_t pos = 0;
			for (auto& code : text)
			{
				const int32_t hash = glyphhash(code, style, params.size);

				if (glyph_lookup.count(hash) == 0)
				{
					// glyph not packed yet, so add to pending list:
					glyphLock.lock();
					pendingGlyphs.insert(hash);
					glyphLock.unlock();
					continue;
				}

				if (code == '\n')
				{
					line += LINEBREAK_SIZE;
					pos = 0;
				}
				else if (code == ' ')
				{
					pos += WHITESPACE_SIZE;
				}
				else if (code == '\t')
				{
					pos += TAB_SIZE;
				}
				else
				{
					const Glyph& glyph = glyph_lookup.at(hash);
					const int16_t glyphWidth = int16_t(glyph.width * params.scaling);
					const int16_t glyphHeight = int16_t(glyph.height * params.scaling);
					const int16_t glyphOffsetX = int16_t(glyph.x * params.scaling);
					const int16_t glyphOffsetY = int16_t(glyph.y * params.scaling);

					const size_t vertexID = size_t(quadCount) * 4;

					const int16_t left = pos + glyphOffsetX;
					const int16_t right = left + glyphWidth;
					const int16_t top = line + glyphOffsetY;
					const int16_t bottom = top + glyphHeight;

					vertexList[vertexID + 0].Pos.x = left;
					vertexList[vertexID + 0].Pos.y = top;
					vertexList[vertexID + 1].Pos.x = right;
					vertexList[vertexID + 1].Pos.y = top;
					vertexList[vertexID + 2].Pos.x = left;
					vertexList[vertexID + 2].Pos.y = bottom;
					vertexList[vertexID + 3].Pos.x = right;
					vertexList[vertexID + 3].Pos.y = bottom;

					vertexList[vertexID + 0].Tex.x = glyph.tc_left;
					vertexList[vertexID + 0].Tex.y = glyph.tc_top;
					vertexList[vertexID + 1].Tex.x = glyph.tc_right;
					vertexList[vertexID + 1].Tex.y = glyph.tc_top;
					vertexList[vertexID + 2].Tex.x = glyph.tc_left;
					vertexList[vertexID + 2].Tex.y = glyph.tc_bottom;
					vertexList[vertexID + 3].Tex.x = glyph.tc_right;
					vertexList[vertexID + 3].Tex.y = glyph.tc_bottom;

					pos += int16_t((glyph.width + params.spacingX) * params.scaling);

					quadCount++;
				}

			}

			return quadCount;
		}

	}
}
using namespace as::asFont_Internal;


asFont::asFont(const std::string& text, asFontParams params, int style) : params(params), style(style)
{
	SetText(text);
}
asFont::asFont(const std::wstring& text, asFontParams params, int style) : params(params), style(style)
{
	SetText(text);
}

void asFont::Initialize()
{
	if (initialized)
	{
		return;
	}

	// add default font if there is none yet:
	if (fontStyles.empty())
	{
		AddFontStyle(FONTPATH + "arial.ttf");
	}

	GraphicsDevice* device = as::asRenderer::GetDevice();

	{
		std::vector<uint16_t> indices(MAX_TEXT * 6);
		for (uint16_t i = 0; i < MAX_TEXT * 4; i += 4)
		{
			indices[size_t(i) / 4 * 6 + 0] = i + 0;
			indices[size_t(i) / 4 * 6 + 1] = i + 2;
			indices[size_t(i) / 4 * 6 + 2] = i + 1;
			indices[size_t(i) / 4 * 6 + 3] = i + 1;
			indices[size_t(i) / 4 * 6 + 4] = i + 2;
			indices[size_t(i) / 4 * 6 + 5] = i + 3;
		}

		GPUBufferDesc bd;
		bd.Usage = USAGE_IMMUTABLE;
		bd.ByteWidth = uint32_t(sizeof(uint16_t) * indices.size());
		bd.BindFlags = BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		SubresourceData InitData;
		InitData.pSysMem = indices.data();

		device->CreateBuffer(&bd, &InitData, &indexBuffer);
	}

	{
		GPUBufferDesc bd;
		bd.Usage = USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(FontCB);
		bd.BindFlags = BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = CPU_ACCESS_WRITE;

		device->CreateBuffer(&bd, nullptr, &constantBuffer);
	}



	RasterizerStateDesc rs;
	rs.FillMode = FILL_SOLID;
	rs.CullMode = CULL_BACK;
	rs.FrontCounterClockwise = true;
	rs.DepthBias = 0;
	rs.DepthBiasClamp = 0;
	rs.SlopeScaledDepthBias = 0;
	rs.DepthClipEnable = false;
	rs.MultisampleEnable = false;
	rs.AntialiasedLineEnable = false;
	device->CreateRasterizerState(&rs, &rasterizerState);

	BlendStateDesc bd;
	bd.RenderTarget[0].BlendEnable = true;
	bd.RenderTarget[0].SrcBlend = BLEND_ONE;
	bd.RenderTarget[0].DestBlend = BLEND_INV_SRC_ALPHA;
	bd.RenderTarget[0].BlendOp = BLEND_OP_ADD;
	bd.RenderTarget[0].SrcBlendAlpha = BLEND_ONE;
	bd.RenderTarget[0].DestBlendAlpha = BLEND_ONE;
	bd.RenderTarget[0].BlendOpAlpha = BLEND_OP_ADD;
	bd.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_ENABLE_ALL;
	bd.IndependentBlendEnable = false;
	device->CreateBlendState(&bd, &blendState);

	DepthStencilStateDesc dsd;
	dsd.DepthEnable = false;
	dsd.StencilEnable = false;
	device->CreateDepthStencilState(&dsd, &depthStencilState);

	SamplerDesc samplerDesc;
	samplerDesc.Filter = FILTER_MIN_MAG_LINEAR_MIP_POINT;
	samplerDesc.AddressU = TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = TEXTURE_ADDRESS_BORDER;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 0;
	samplerDesc.ComparisonFunc = COMPARISON_NEVER;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = FLT_MAX;
	device->CreateSamplerState(&samplerDesc, &sampler);

	LoadShaders();

	as::asBackLog::post("asFont Initialized");
	initialized.store(true);
}

void asFont::LoadShaders()
{
	std::string path = as::asRenderer::GetShaderPath();

	VertexLayoutDesc layout[] =
	{
		{ "POSITION", 0, FORMAT_R16G16_SINT, 0, VertexLayoutDesc::APPEND_ALIGNED_ELEMENT, INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, FORMAT_R16G16_FLOAT, 0, VertexLayoutDesc::APPEND_ALIGNED_ELEMENT, INPUT_PER_VERTEX_DATA, 0 },
	};
	as::asRenderer::LoadVertexShader(vertexShader, "fontVS.cso");
	as::asRenderer::GetDevice()->CreateInputLayout(layout, arraysize(layout), &vertexShader.code, &vertexLayout);


	as::asRenderer::LoadPixelShader(pixelShader, "fontPS.cso");


	PipelineStateDesc desc;
	desc.vs = &vertexShader;
	desc.ps = &pixelShader;
	desc.il = &vertexLayout;
	desc.bs = &blendState;
	desc.dss = &depthStencilState;
	desc.rs = &rasterizerState;
	as::asRenderer::GetDevice()->CreatePipelineState(&desc, &PSO);
}

void UpdatePendingGlyphs()
{
	// If there are pending glyphs, render them and repack the atlas:
	if (!pendingGlyphs.empty())
	{
		// Pad the glyph rects in the atlas to avoid bleeding from nearby texels:
		const int borderPadding = 1;

		for (int32_t hash : pendingGlyphs)
		{
			const int code = codefromhash(hash);
			const int style = stylefromhash(hash);
			const int height = heightfromhash(hash);
			asFontStyle& fontStyle = fontStyles[style];

			float fontScaling = stbtt_ScaleForPixelHeight(&fontStyle.fontInfo, float(height));

			// get bounding box for character (may be offset to account for chars that dip above or below the line
			int left, top, right, bottom;
			stbtt_GetCodepointBitmapBox(&fontStyle.fontInfo, code, fontScaling, fontScaling, &left, &top, &right, &bottom);

			// Glyph dimensions are calculated without padding:
			Glyph& glyph = glyph_lookup[hash];
			glyph.x = left;
			glyph.y = top + int(fontStyle.ascent * fontScaling);
			glyph.width = right - left;
			glyph.height = bottom - top;

			// Add padding to the rectangle that will be packed in the atlas:
			right += borderPadding * 2;
			bottom += borderPadding * 2;
			rect_lookup[hash] = rect_ltrb(left, top, right, bottom);
		}
		pendingGlyphs.clear();

		// This reference array will be used for packing:
		vector<rect_xywh*> out_rects;
		out_rects.reserve(rect_lookup.size());
		for (auto& it : rect_lookup)
		{
			out_rects.push_back(&it.second);
		}

		// Perform packing and process the result if successful:
		std::vector<bin> bins;
		if (pack(out_rects.data(), (int)out_rects.size(), 4096, bins))
		{
			assert(bins.size() == 1 && "The regions won't fit into one texture!");

			// Retrieve texture atlas dimensions:
			const int bitmapWidth = bins[0].size.w;
			const int bitmapHeight = bins[0].size.h;
			const float inv_width = 1.0f / bitmapWidth;
			const float inv_height = 1.0f / bitmapHeight;

			// Create the CPU-side texture atlas and fill with transparency (0):
			vector<uint8_t> bitmap(size_t(bitmapWidth) * size_t(bitmapHeight));
			std::fill(bitmap.begin(), bitmap.end(), 0);

			// Iterate all packed glyph rectangles:
			for (auto it : rect_lookup)
			{
				const int32_t hash = it.first;
				const wchar_t code = codefromhash(hash);
				const int style = stylefromhash(hash);
				const int height = heightfromhash(hash);
				asFontStyle& fontStyle = fontStyles[style];
				rect_xywh& rect = it.second;
				Glyph& glyph = glyph_lookup[hash];

				// Remove border padding from the packed rectangle (we don't want to touch the border, it should stay transparent):
				rect.x += borderPadding;
				rect.y += borderPadding;
				rect.w -= borderPadding * 2;
				rect.h -= borderPadding * 2;

				float fontScaling = stbtt_ScaleForPixelHeight(&fontStyle.fontInfo, float(height));

				// Render the glyph inside the CPU-side atlas:
				int byteOffset = rect.x + (rect.y * bitmapWidth);
				stbtt_MakeCodepointBitmap(&fontStyle.fontInfo, bitmap.data() + byteOffset, rect.w, rect.h, bitmapWidth, fontScaling, fontScaling, code);

				// Compute texture coordinates for the glyph:
				float tc_left = float(rect.x);
				float tc_right = tc_left + float(rect.w);
				float tc_top = float(rect.y);
				float tc_bottom = tc_top + float(rect.h);

				tc_left *= inv_width;
				tc_right *= inv_width;
				tc_top *= inv_height;
				tc_bottom *= inv_height;

				glyph.tc_left = XMConvertFloatToHalf(tc_left);
				glyph.tc_right = XMConvertFloatToHalf(tc_right);
				glyph.tc_top = XMConvertFloatToHalf(tc_top);
				glyph.tc_bottom = XMConvertFloatToHalf(tc_bottom);

			}

			// Upload the CPU-side texture atlas bitmap to the GPU:
			as::asTextureHelper::CreateTexture(texture, bitmap.data(), bitmapWidth, bitmapHeight, FORMAT_R8_UNORM);
		}
	}
}
const Texture* asFont::GetAtlas()
{
	return &texture;
}
const std::string& asFont::GetFontPath()
{
	return FONTPATH;
}
void asFont::SetFontPath(const std::string& path)
{
	FONTPATH = path;
}
int asFont::AddFontStyle(const string& fontName)
{
	for (size_t i = 0; i < fontStyles.size(); i++)
	{
		const asFontStyle& fontStyle = fontStyles[i];
		if (!fontStyle.name.compare(fontName))
		{
			return int(i);
		}
	}
	fontStyles.emplace_back();
	fontStyles.back().Create(fontName);
	return int(fontStyles.size() - 1);
}


void asFont::Draw(CommandList cmd) const
{
	if (!initialized.load() || text.length() <= 0)
	{
		return;
	}

	asFontParams newProps = params;

	if (params.h_align == WIFALIGN_CENTER)
		newProps.posX -= textWidth() / 2;
	else if (params.h_align == WIFALIGN_RIGHT)
		newProps.posX -= textWidth();
	if (params.v_align == WIFALIGN_CENTER)
		newProps.posY -= textHeight() / 2;
	else if (params.v_align == WIFALIGN_BOTTOM)
		newProps.posY -= textHeight();



	GraphicsDevice* device = as::asRenderer::GetDevice();

	GraphicsDevice::GPUAllocation mem = device->AllocateGPU(sizeof(FontVertex) * text.length() * 4, cmd);
	if (!mem.IsValid())
	{
		return;
	}
	volatile FontVertex* textBuffer = (volatile FontVertex*)mem.data;
	const int quadCount = WriteVertices(textBuffer, text, newProps, style);

	device->EventBegin("Font", cmd);

	device->BindPipelineState(&PSO, cmd);

	device->BindConstantBuffer(VS, &constantBuffer, CB_GETBINDSLOT(FontCB), cmd);
	device->BindConstantBuffer(PS, &constantBuffer, CB_GETBINDSLOT(FontCB), cmd);
	device->BindResource(PS, &texture, TEXSLOT_FONTATLAS, cmd);
	device->BindSampler(PS, &sampler, SSLOT_ONDEMAND1, cmd);

	const GPUBuffer* vbs[] = {
		mem.buffer,
	};
	const uint32_t strides[] = {
		sizeof(FontVertex),
	};
	const uint32_t offsets[] = {
		mem.offset,
	};
	device->BindVertexBuffers(vbs, 0, arraysize(vbs), strides, offsets, cmd);

	assert(text.length() * 4 < 65536 && "The index buffer currently only supports so many characters!");
	device->BindIndexBuffer(&indexBuffer, INDEXFORMAT_16BIT, 0, cmd);

	FontCB cb;

	if (newProps.shadowColor.getA() > 0)
	{
		// font shadow render:
		XMStoreFloat4x4(&cb.g_xFont_Transform,
			XMMatrixTranslation((float)newProps.posX + 1, (float)newProps.posY + 1, 0)
			* device->GetScreenProjection()
		);
		cb.g_xFont_Color = newProps.shadowColor.toFloat4();
		device->UpdateBuffer(&constantBuffer, &cb, cmd);

		device->DrawIndexed(quadCount * 6, 0, 0, cmd);
	}

	// font base render:
	XMStoreFloat4x4(&cb.g_xFont_Transform,
		XMMatrixTranslation((float)newProps.posX, (float)newProps.posY, 0)
		* device->GetScreenProjection()
	);
	cb.g_xFont_Color = newProps.color.toFloat4();
	device->UpdateBuffer(&constantBuffer, &cb, cmd);

	device->DrawIndexed(quadCount * 6, 0, 0, cmd);

	device->EventEnd(cmd);

	UpdatePendingGlyphs();
}


int asFont::textWidth() const
{
	if (style >= (int)fontStyles.size())
	{
		return 0;
	}

	int maxWidth = 0;
	int currentLineWidth = 0;
	for (auto& code : text)
	{
		const int32_t hash = glyphhash(code, style, params.size);

		if (glyph_lookup.count(hash) == 0)
		{
			// glyph not packed yet, we just continue (it will be added if it is actually rendered)
			continue;
		}

		if (code == '\n')
		{
			currentLineWidth = 0;
		}
		else if (code == ' ')
		{
			currentLineWidth += WHITESPACE_SIZE;
		}
		else if (code == '\t')
		{
			currentLineWidth += TAB_SIZE;
		}
		else
		{
			const Glyph& glyph = glyph_lookup.at(hash);
			currentLineWidth += int((glyph.width + params.spacingX) * params.scaling);
		}
		maxWidth = std::max(maxWidth, currentLineWidth);
	}

	return maxWidth;
}
int asFont::textHeight() const
{
	if (style >= (int)fontStyles.size())
	{
		return 0;
	}

	int height = LINEBREAK_SIZE;
	for (auto& code : text)
	{
		if (code == '\n')
		{
			height += LINEBREAK_SIZE;
		}
	}

	return height;
}

void asFont::SetText(const string& text)
{
	as::asHelper::StringConvert(text, this->text);
}
void asFont::SetText(const wstring& text)
{
	this->text = text;
}
wstring asFont::GetText() const
{
	return text;
}
string asFont::GetTextA() const
{
	string retval;
	as::asHelper::StringConvert(this->text, retval);
	return retval;
}
