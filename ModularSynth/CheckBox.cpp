#include "CheckBox.h"

constexpr float checkBoxSize = 20.0f;
constexpr float checkBoxMarkSize = checkBoxSize - 5.0f;
constexpr float checkBoxTextGap = 5.0f;
constexpr float dur = 0.08f;

const float checkMarkCoords[] = {
	10.0f, 60.0f,
	40.0f, 90.0f,
	90.0f, 30.0f,
	90.0f, 15.0f,
	40.0f, 75.0f,
	10.0f, 45.0f
};

void CheckBox::onDraw(NVGcontext* ctx, float deltaTime) {
	float hoverValue = m_hoverAnimator.value(Curves::easeInOutQuad, deltaTime);
	float clickValue = m_clickAnimator.value(Curves::easeInOutQuad, deltaTime);
	float checkValue = m_checkAnimator.value(Curves::easeInOutQuad, deltaTime);

	float clickAlphaValue = std::lerp(0.6f, 1.0f, clickValue);
	float clickBgValue = std::lerp(0.0f, 1.0f, clickValue);
	float clickFgValue = 1.0f - clickBgValue;

	Rect b = bounds;

	nvgBeginPath(ctx);
	nvgRoundedRect(ctx, 0.0f, b.height / 2 - checkBoxSize / 2.0f, checkBoxSize, checkBoxSize, 5.0f);
	nvgFillColor(ctx, nvgRGBAf(clickBgValue, clickBgValue, clickBgValue, clickAlphaValue));
	if (hoverValue >= 1e-5f) {
		nvgStrokeColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, hoverValue));
		nvgStroke(ctx);
	}
	nvgFill(ctx);

	nvgFillColor(ctx, nvgRGBf(1.0f, 1.0f, 1.0f));
	nvgTextAlign(ctx, NVG_ALIGN_MIDDLE);
	nvgText(ctx, checkBoxSize + checkBoxTextGap, b.height / 2 + 1.5f, text.c_str(), nullptr);

	if (checkValue > 0.0f) {
		nvgBeginPath(ctx);
		nvgScissor(ctx,
			checkBoxSize / 2.0f - checkBoxMarkSize / 2.0f,
			b.height / 2 - checkBoxMarkSize / 2.0f,
			checkValue * checkBoxMarkSize,
			checkBoxMarkSize
		);
		for (size_t i = 0; i < std::size(checkMarkCoords); i += 2) {
			float cx = (checkMarkCoords[i] / 100.0f) * checkBoxMarkSize;
			float cy = (checkMarkCoords[i + 1] / 100.0f) * checkBoxMarkSize;

			cx += checkBoxSize / 2.0f - checkBoxMarkSize / 2.0f;
			cy += b.height / 2 - checkBoxMarkSize / 2.0f;

			if (i == 0) {
				nvgMoveTo(ctx, cx, cy);
			}
			else {
				nvgLineTo(ctx, cx, cy);
			}
		}
		nvgFillColor(ctx, nvgRGBAf(clickFgValue, clickFgValue, clickFgValue, 1.0f));
		nvgFill(ctx);
	}
}

void CheckBox::onMouseUp(int button, int x, int y) {
	m_clickAnimator.target(0.0f, dur);
	if (button == 1) {
		if (!m_selected) {
			m_checkAnimator.target(1.0f, 0.25f);
		}
		else {
			m_checkAnimator.target(0.0f, 0.25f);
		}
		m_selected = !m_selected;
	}
}

void CheckBox::onMouseDown(int button, int x, int y) {
	m_clickAnimator.target(1.0f, 0.25f);
	m_hoverAnimator.target(0.0f, dur);
}

void CheckBox::onMouseEnter() {
	m_hoverAnimator.target(1.0f, dur);
	m_clickAnimator.target(0.0f, 0.01f);
}

void CheckBox::onMouseLeave() {
	m_hoverAnimator.target(0.0f, dur);
	m_clickAnimator.target(0.0f, 0.01f);
}
