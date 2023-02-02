#pragma once

#include "Window.h"
#include "nanovg/nanovg.h"

#include <map>
#include <array>
#include <cmath>
#include <memory>
#include <functional>
#include <string>
#include <regex>

class LineEditor {
public:
	void drawText(NVGcontext* ctx, float deltaTime);
	void drawCursor(NVGcontext* ctx, float deltaTime);
	bool keyPress(int keyCode);
	bool keyRelease(int keyCode);
	bool type(TCHAR charCode);

	void mouseDown(int button, int x, int y, float height);
	void blur();

	std::string text{ "" };
	std::regex inputFilter{ ".?" };

	std::function<void(const std::string&)> onChange, onEditingComplete;

private:
	size_t m_cursorX{ 0 };

	bool m_cursorShow{ true }, m_ctrl{ false };
	float m_blinkTimer{ 0.0f };

	std::vector<NVGglyphPosition> m_glyphs;

	void resetCursor();
};
