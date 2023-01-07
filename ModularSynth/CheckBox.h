#pragma once

#include "Control.h"
#include "Animator.h"
#include <string>

class CheckBox : public Control {
public:
	void onDraw(NVGcontext* ctx, float deltaTime) override;

	void onMouseUp(int button, int x, int y);
	void onMouseDown(int button, int x, int y);
	void onMouseEnter();
	void onMouseLeave();

	std::string text{ "Check Box" };
	float fontSize{ 16.0f };

	bool selected() const { return m_selected; }
	void selected(bool state) { m_selected = state; }

private:
	bool m_selected{ false };

	Animator<float> m_hoverAnimator{};
	Animator<float> m_clickAnimator{};
	Animator<float> m_checkAnimator{};
};

