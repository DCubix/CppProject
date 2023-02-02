#pragma once

#include "Control.h"
#include "LineEditor.h"

class ValueEdit : public Control, public LineEditor {
public:
	void onDraw(NVGcontext* ctx, float deltaTime) override;
	bool onKeyPress(int keyCode) override;
	bool onKeyRelease(int keyCode) override;
	bool onType(TCHAR charCode) override;

	void onMouseDoubleClick(int button, int x, int y) override;

	void onMouseDown(int button, int x, int y) override;
	void onMouseUp(int button, int x, int y) override;
	void onMouseMove(int x, int y, int dx, int dy) override;
	void onMouseLeave() override;
	void onBlur() override;

	std::string label{ "" }, valueFormat{ "{:.2f}" };
	float value{ 0.0f }, step{ 0.1f };

	std::function<void(float)> onValueChange{ nullptr };

private:
	float m_labelOffset{ 0.0f };
	bool m_editingText{ false }, m_dragging{ false };

	void calculateValue(int mx);
	void textToValue();
};
