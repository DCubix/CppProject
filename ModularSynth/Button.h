#pragma once

#include "Control.h"
#include "Animator.h"
#include <string>

class Button : public Control {
public:
	void onDraw(NVGcontext* ctx, float deltaTime) override;

	void onMouseDown(int button, int x, int y) override;
	void onMouseUp(int button, int x, int y) override;
	void onMouseEnter() override;
	void onMouseLeave() override;

	std::function<void()> onPress{ nullptr };
	std::string text{ "Label" };
	size_t icon{ 0 };
	float fontSize{ 15.0f };

private:
	Animator<float> m_hoverAnimator{};
	Animator<float> m_clickAnimator{};

};

