#include "Edit.h"

#include <atlstr.h>

constexpr float textPad = 8.0f;

void Edit::onDraw(NVGcontext* ctx, float deltaTime) {
	Rect b = bounds;
	nvgBeginPath(ctx);
	nvgRoundedRect(ctx, 0, 0, b.width, b.height, 12.0f);
	nvgFillColor(ctx, nvgRGBAf(0.0f, 0.0f, 0.0f, 0.6f));
	if (m_focused) {
		nvgStrokeColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
		nvgStroke(ctx);
	}
	nvgFill(ctx);

	m_labelOffset = 0.0f;
	if (!label.empty()) {
		float lblBounds[4] = { 0.0f };
		nvgTextBounds(ctx, 0.0f, 0.0f, label.c_str(), nullptr, lblBounds);
		m_labelOffset = lblBounds[2] - lblBounds[0];

		nvgFillColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 0.5f));
		nvgText(ctx, textPad, b.height / 2 + 1.5f, label.c_str(), nullptr);
	}

	nvgSave(ctx);
	nvgTranslate(ctx, textPad + m_labelOffset, b.height / 2.0f);
	drawText(ctx, deltaTime);

	// draw cursor
	if (m_focused) {
		drawCursor(ctx, deltaTime);
	}
	nvgRestore(ctx);
}

bool Edit::onKeyPress(int keyCode) {
	if (!m_focused) return false;
	return keyPress(keyCode);
}

bool Edit::onKeyRelease(int keyCode) {
	return keyRelease(keyCode);
}

void Edit::onMouseDown(int button, int x, int y) {
	mouseDown(button, x - m_labelOffset, y, bounds.height);
}

void Edit::onBlur() {
	blur();
}

bool Edit::onType(TCHAR charCode) {
	if (m_focused) {
		return type(charCode);
	}
	return false;
}
