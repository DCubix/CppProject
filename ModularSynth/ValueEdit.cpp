#include "ValueEdit.h"

#include <algorithm>
#include <format>

constexpr float textPad = 8.0f;
constexpr float labelGap = 2.0f;
constexpr float sensitivity = 0.2f;

static void drawArrow(NVGcontext* ctx, float x, float y, float radius, float facing, float r, float g, float b, float a) {
	float x1 = x - radius * facing * 0.8f, y1 = y - radius,
		x2 = x + radius * facing * 0.8f, y2 = y,
		x3 = x - radius * facing * 0.8f, y3 = y + radius;

	nvgBeginPath(ctx);
	nvgMoveTo(ctx, x1, y1);
	nvgLineTo(ctx, x2, y2);
	nvgLineTo(ctx, x3, y3);
	nvgClosePath(ctx);
	nvgFillColor(ctx, nvgRGBAf(r, g, b, a));
	nvgFill(ctx);
}

void ValueEdit::onDraw(NVGcontext* ctx, float deltaTime) {
	// hack to lock the input filter
	inputFilter = std::regex("[0-9\\.\\-]");

	Rect b = bounds;
	nvgBeginPath(ctx);
	nvgRoundedRect(ctx, 0, 0, b.width, b.height, 12.0f);
	nvgFillColor(ctx, nvgRGBAf(0.0f, 0.0f, 0.0f, 0.6f));
	if (m_focused) {
		nvgStrokeColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
		nvgStroke(ctx);
	}
	nvgFill(ctx);
	
	nvgFontSize(ctx, 16.0f);

	nvgScissor(ctx, 12.0f, 2.0f, b.width - 24.0f, b.height - 4.0f);

	std::string valueText = std::vformat(valueFormat, std::make_format_args(m_value));

	m_labelOffset = textPad;
	if (!m_editingText) {
		std::string allText = label + valueText;
		float txtBounds[4] = { 0.0f };
		nvgTextBounds(ctx, 0.0f, 0.0f, allText.c_str(), nullptr, txtBounds);
		m_labelOffset = b.width / 2 - (txtBounds[2] - txtBounds[0]) / 2;
	}

	if (!label.empty()) {
		float lblBounds[4] = { 0.0f };
		nvgTextBounds(ctx, 0.0f, 0.0f, label.c_str(), nullptr, lblBounds);

		nvgFillColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 0.5f));
		nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
		nvgText(ctx, m_labelOffset, b.height / 2 + 1.5f, label.c_str(), nullptr);

		m_labelOffset += (lblBounds[2] - lblBounds[0]) + labelGap;
	}

	if (m_editingText) {
		nvgSave(ctx);
		nvgTranslate(ctx, m_labelOffset, b.height / 2.0f);
		drawText(ctx, deltaTime);

		// draw cursor
		if (m_focused) {
			drawCursor(ctx, deltaTime);
		}
		nvgRestore(ctx);

		nvgResetScissor(ctx);
	}
	else {
		nvgFillColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
		nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
		nvgText(ctx, m_labelOffset, b.height / 2 + 1.5f, valueText.c_str(), nullptr);

		nvgResetScissor(ctx);

		// arrows
		const float radius = (b.height / 2) - 2;
		drawArrow(ctx, radius, b.height / 2, 4.0f, -1.0f, 0.5f, 0.5f, 0.5f, 0.7f);
		drawArrow(ctx, b.width - radius, b.height / 2, 4.0f, 1.0f, 0.5f, 0.5f, 0.5f, 0.7f);
	}

}

bool ValueEdit::onKeyPress(int keyCode) {
	if (m_editingText) {
		if (keyCode == VK_RETURN) {
			textToValue();
			return true;
		}

		return keyPress(keyCode);
	}
	return false;
}

bool ValueEdit::onKeyRelease(int keyCode) {
	if (m_editingText) {
		return keyRelease(keyCode);
	}
	return false;
}

bool ValueEdit::onType(TCHAR charCode) {
	if (m_editingText) {
		return type(charCode);
	}
	return false;
}

void ValueEdit::onMouseDown(int button, int x, int y) {
	if (m_editingText) {
		mouseDown(button, x, y, bounds.height);
		return;
	}

	if (button != 1) return;

	m_dragging = true;
}

void ValueEdit::onMouseUp(int button, int x, int y) {
	m_dragging = false;
}

void ValueEdit::onMouseMove(int x, int y, int dx, int dy) {
	if (m_dragging) {
		calculateValue(dx);
	}
}

void ValueEdit::onMouseLeave() {
	m_dragging = false;
}

void ValueEdit::onBlur() {
	if (m_editingText) {
		blur();
		textToValue();
	}
	m_dragging = false;
}

float ValueEdit::snap(float v) {
	float newStep = ::roundf(v / step);
	float newValue = newStep * step;
	return newValue;
}

void ValueEdit::calculateValue(int dx) {
	m_actualValue += float(dx) * step * sensitivity;

	float newValue = snap(m_actualValue);

	if (newValue != m_value) {
		value(newValue);
	}
}

void ValueEdit::textToValue() {
	m_editingText = false;
	value(std::stof(text));
	if (onEditingComplete) onEditingComplete(text);
}

void ValueEdit::onMouseDoubleClick(int button, int x, int y) {
	if (button == 1 && !m_editingText) {
		m_editingText = true;
		text = std::to_string(m_value);
	}
}
