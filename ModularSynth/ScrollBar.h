#pragma once 

#include "Control.h"
#include "Animator.h"
#include <string>


enum class SBOrientation {
	vertical,
	horizontal
};

class ScrollBar : public Control {
public:

	void onDraw(NVGcontext* ctx, float deltaTime) override;
	void onPostDraw(NVGcontext* ctx, float deltaTime) override;

	void onMouseDown(int button, int x, int y) override;
	void onMouseUp(int button, int x, int y) override;
	void onMouseMove(int x, int y, int dx, int dy) override;
	void onMouseEnter() override;
	void onMouseLeave() override;

	std::function<void(float)> onChange{ nullptr };

	float pageMin, pageMax, page, pageSize, pageStep;
	float stepTimer = 0.f;
	SBOrientation orientation = SBOrientation::horizontal;

private:
	Animator<float> m_anim{};

	Point m_mousePos;
	Point m_grabCoords;
	bool m_active;
	bool m_dragging{ false };
	bool m_increasing{ false };
	bool m_decreasing{ false };

	void calculateValue(int delta);

	Rect handleArea();
	Rect handleRect();
	Rect negativeQuickScrollArea();
	Rect positiveQuickScrollArea();
};
