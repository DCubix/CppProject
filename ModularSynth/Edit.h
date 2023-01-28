#pragma once

#include "Control.h"

#include <functional>
#include <string>
#include <regex>

class Edit : public Control {
public:
	void onDraw(NVGcontext* ctx, float deltaTime) override;
	void onKeyPress(int keyCode) override;
	bool onType(TCHAR charCode) override;

	void onMouseDown(int button, int x, int y) override;

	std::string text{ "" }, label{ "" };
	std::regex inputFilter{ ".?" };

	std::function<void(const std::string&)> onChange;

private:
	size_t m_cursorX{ 0 };

	bool m_cursorShow{ true };
	float m_blinkTimer{ 0.0f };

	void resetCursor();

	std::vector<NVGglyphPosition> m_glyphs;
	float m_labelOffset{ 0.0f };

};

