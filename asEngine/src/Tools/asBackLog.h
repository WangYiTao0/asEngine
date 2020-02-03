#pragma once
#include "CommonInclude.h"
#include "Graphics/API/asGraphicsDevice.h"

#include <string>
#include <fstream>

namespace asBackLog
{
	void Toggle();
	void Scroll(int direction);
	void Update();
	void Draw(asGraphics::CommandList cmd);

	std::string getText();
	void clear();
	void post(const char* input);
	void input(const char& input);
	void acceptInput();
	void deletefromInput();
	void save(std::ofstream& file);

	void historyPrev();
	void historyNext();

	bool isActive();

	void setBackground(asGraphics::Texture* texture);
	void setFontSize(int value);
	void setFontRowspacing(int value);
};
