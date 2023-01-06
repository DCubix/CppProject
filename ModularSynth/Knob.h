#pragma once

#include "Control.h"
#include <string>
#include <functional>

class Knob : public Control {
public:
	void onDraw(NVGcontext* ctx, float deltaTime) override;

	void onMouseDown(int button, int x, int y) override;
	void onMouseUp(int button, int x, int y) override;
	void onMouseMove(int x, int y, int dx, int dy) override;
	void onMouseLeave() override;

	std::function<void(float)> onChange{ nullptr };
	std::string valueFormat{ "{:.2f}" };

	float min{ 0.0f }, max{ 1.0f }, step{ 0.1f }, value{ 0.5f };

private:
	bool m_dragging{ false };

	void calculateValue(int x, int y);
};

