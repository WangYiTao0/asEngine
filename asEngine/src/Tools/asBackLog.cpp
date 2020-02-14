#include "aspch.h"
#include "asBackLog.h"

#include "Helpers/asMath.h"
#include "Helpers/asResourceManager.h"
#include "Graphics/asRenderer.h"
#include "Graphics/asTextureHelper.h"
#include "Helpers/asSpinLock.h"
#include "Graphics/asFont.h"
#include "Graphics/asImage.h"


#include <mutex>
#include <sstream>
#include <deque>

namespace as
{

	using namespace std;
	using namespace asGraphics;


	namespace asBackLog
	{

		enum State {
			DISABLED,
			IDLE,
			ACTIVATING,
			DEACTIVATING,
		};

		deque<string> stream;
		deque<string> history;
		State state = DISABLED;
		const float speed = 50.0f;
		unsigned int deletefromline = 100;
		float pos = -FLT_MAX;
		int scroll = 0;
		stringstream inputArea;
		int historyPos = 0;
		asFont font;
		asSpinLock logLock;

		std::unique_ptr<Texture> backgroundTex;

		void Toggle()
		{
			switch (state)
			{
			case IDLE:
				state = DEACTIVATING;
				break;
			case DISABLED:
				state = ACTIVATING;
				break;
			default:break;
			};
		}
		void Scroll(int dir)
		{
			scroll += dir;
		}
		void Update()
		{
			if (state == DEACTIVATING)
				pos -= speed;
			else if (state == ACTIVATING)
				pos += speed;
			if (pos <= -asRenderer::GetDevice()->GetScreenHeight())
			{
				state = DISABLED;
				pos = -(float)asRenderer::GetDevice()->GetScreenHeight();
			}
			else if (pos > 0)
			{
				state = IDLE;
				pos = 0;
			}

			if (scroll + font.textHeight() > int(asRenderer::GetDevice()->GetScreenHeight() * 0.8f))
			{
				scroll -= 2;
			}
		}
		void Draw(CommandList cmd)
		{
			if (state != DISABLED)
			{
				if (backgroundTex == nullptr)
				{
					const uint8_t colorData[] = { 0, 0, 43, 200, 43, 31, 141, 223 };
					backgroundTex.reset(new Texture);
					asTextureHelper::CreateTexture(*backgroundTex.get(), colorData, 1, 2);
				}

				asImageParams fx = asImageParams((float)asRenderer::GetDevice()->GetScreenWidth(), (float)asRenderer::GetDevice()->GetScreenHeight());
				fx.pos = XMFLOAT3(0, pos, 0);
				fx.opacity = asMath::Lerp(1, 0, -pos / asRenderer::GetDevice()->GetScreenHeight());
				asImage::Draw(backgroundTex.get(), fx, cmd);
				font.SetText(getText());
				font.params.posX = 50;
				font.params.posY = (int)pos + (int)scroll;
				font.Draw(cmd);
				asFont(inputArea.str().c_str(), asFontParams(10, asRenderer::GetDevice()->GetScreenHeight() - 10, ASFONTSIZE_DEFAULT, ASFALIGN_LEFT, ASFALIGN_BOTTOM)).Draw(cmd);
			}
		}


		string getText()
		{
			logLock.lock();
			stringstream ss("");
			for (unsigned int i = 0; i < stream.size(); ++i)
				ss << stream[i];
			logLock.unlock();
			return ss.str();
		}
		void clear()
		{
			logLock.lock();
			stream.clear();
			logLock.unlock();
		}
		void post(const char* input)
		{
			logLock.lock();
			stringstream ss("");
			ss << input << endl;
			stream.push_back(ss.str().c_str());
			if (stream.size() > deletefromline) {
				stream.pop_front();
			}
			logLock.unlock();
		}
		void input(const char& input)
		{
			inputArea << input;
		}
		void acceptInput()
		{
			historyPos = 0;
			stringstream commandStream("");
			commandStream << inputArea.str();
			post(inputArea.str().c_str());
			history.push_back(inputArea.str());
			if (history.size() > deletefromline) {
				history.pop_front();
			}
			//wiLua::GetGlobal()->RunText(inputArea.str());
			inputArea.str("");
		}
		void deletefromInput()
		{
			stringstream ss(inputArea.str().substr(0, inputArea.str().length() - 1));
			inputArea.str("");
			inputArea << ss.str();
		}
		void save(ofstream& file)
		{
			for (deque<string>::iterator iter = stream.begin(); iter != stream.end(); ++iter)
				file << iter->c_str();
			file.close();
		}

		void historyPrev()
		{
			if (!history.empty())
			{
				inputArea.str("");
				inputArea << history[history.size() - 1 - historyPos];
				if ((size_t)historyPos < history.size() - 1)
					historyPos++;
			}
		}
		void historyNext()
		{
			if (!history.empty())
			{
				if (historyPos > 0)
					historyPos--;
				inputArea.str("");
				inputArea << history[history.size() - 1 - historyPos];
			}
		}

		void setBackground(Texture* texture)
		{
			backgroundTex.reset(texture);
		}
		void setFontSize(int value)
		{
			font.params.size = value;
		}
		void setFontRowspacing(int value)
		{
			font.params.spacingY = value;
		}

		bool isActive() { return state == IDLE; }

	}

}

