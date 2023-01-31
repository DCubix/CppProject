#pragma once

#include "Control.h"

#include <array>

class ColorWheel : public Control {
public:
	void onDraw(NVGcontext* ctx, float deltaTime);
	void onMouseMove(int x, int y, int dx, int dy) override;
	void onMouseDown(int button, int x, int y) override;
	void onMouseUp(int button, int x, int y) override;

	float hue{ 0.0f }, saturation{ 1.0f }, value{ 1.0f };

private:
	enum DragState {
		idling = 0,
		hueRing,
		satValTriangle
	} state{ idling };

	std::array<float, 6> m_triangle;

	Point m_triangleSelector{ 0.0f, 0.0f };

	float m_w{ 0.0f };

	void updateHue(int x, int y);
	void updateSatVal(int x, int y);
};
