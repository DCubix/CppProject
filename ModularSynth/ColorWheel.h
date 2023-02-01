#pragma once

#include "Control.h"

#include <array>
#include <functional>

class ColorWheel : public Control {
public:
	void onDraw(NVGcontext* ctx, float deltaTime);
	void onMouseMove(int x, int y, int dx, int dy) override;
	void onMouseDown(int button, int x, int y) override;
	void onMouseUp(int button, int x, int y) override;
	void onMouseLeave() override;

	Color color() const { return m_color; }
	void color(Color col);

	std::function<void(const Color&)> onChange{ nullptr };

private:
	enum DragState {
		idling = 0,
		hueRing,
		satVal
	} state{ idling };

	Color m_color{ 0.0f, 0.0f, 0.0f, 1.0f };
	Point m_selector{ 0.0f, 0.0f };

	std::array<float, 3> m_hsv;

	void updateHue(int x, int y);
	void updateSatVal(int x, int y);
	void updateColor();
	void updatePoint();
};
