#pragma once

#include "Control.h"
#include "LineEditor.h"

#include <functional>
#include <string>
#include <regex>

class Edit : public Control, public LineEditor {
public:
	void onDraw(NVGcontext* ctx, float deltaTime) override;
	bool onKeyPress(int keyCode) override;
	bool onKeyRelease(int keyCode) override;
	bool onType(TCHAR charCode) override;

	void onMouseDown(int button, int x, int y) override;
	void onBlur() override;

	std::string label{ "" };
private:
	float m_labelOffset{ 0.0f };
};

