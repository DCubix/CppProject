#include "Button.h"

#include "Icons.hpp"

constexpr float dur = 0.08f;

void Button::onDraw(NVGcontext* ctx, float deltaTime) {
	float hoverValue = m_hoverAnimator.value(Curves::easeInOutQuad, deltaTime);
	float clickValue = m_clickAnimator.value(Curves::easeInOutQuad, deltaTime);

	float clickAlphaValue = std::lerp(0.6f, 1.0f, clickValue);
	float clickBgValue = std::lerp(0.0f, 1.0f, clickValue);
	float clickFgValue = 1.0f - clickBgValue;

	float scl = clickValue * 2.0f;

	Rect b = bounds;
	nvgBeginPath(ctx);
	nvgRoundedRect(ctx, scl, scl, b.width - scl * 2, b.height - scl * 2, 12.0f);
	nvgFillColor(ctx, nvgRGBAf(clickBgValue, clickBgValue, clickBgValue, clickAlphaValue));
	if (hoverValue >= 1e-5f) {
		nvgStrokeColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, hoverValue));
		nvgStroke(ctx);
	}
	nvgFill(ctx);

	float textOffset = 0.0f;
	if (icon > 0) {
		auto ico = icons[icon];
		textOffset += ico.viewBoxWidth / 2 + 3.0f;

		float txtBounds[4] = { 0.0f };
		nvgTextBounds(ctx, 0.0f, 0.0f, text.c_str(), nullptr, txtBounds);
		float tw = (txtBounds[2] - txtBounds[0]);

		ico.render(ctx, b.width / 2 - (tw / 2 + 3.0f), b.height / 2, clickFgValue, clickFgValue, clickFgValue);
	}

	nvgFillColor(ctx, nvgRGBf(clickFgValue, clickFgValue, clickFgValue));
	nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
	nvgFontSize(ctx, 16.0f);
	nvgTextBox(ctx, textOffset, b.height / 2 + 1.5f, b.width, text.c_str(), nullptr);
}

void Button::onMouseDown(int button, int x, int y) {
	m_clickAnimator.target(1.0f, dur);
}

void Button::onMouseUp(int button, int x, int y) {
	m_clickAnimator.target(0.0f, dur);
	if (button == 1 && onPress) {
		onPress();
	}
}

void Button::onMouseEnter() {
	m_hoverAnimator.target(1.0f, dur);
	m_clickAnimator.target(0.0f, dur);
}

void Button::onMouseLeave() {
	m_hoverAnimator.target(0.0f, dur);
	m_clickAnimator.target(0.0f, dur);
}
