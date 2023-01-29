#include "Slider.h"

#include <format>
#include <iostream>

constexpr float sliderHeight = 20.0f;
constexpr float popupHeignt = 18.0f;

static void drawArrow(NVGcontext* ctx,  float x, float y, float radius, float facing, float r, float g, float b, float a) {
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

void Slider::onDraw(NVGcontext* ctx, float deltaTime) {
	Rect b = bounds;
	Rect sliderRect = { 0, b.height / 2 - sliderHeight / 2, b.width, sliderHeight };
	const float radius = (sliderHeight / 2) - 2;

	// track
	nvgBeginPath(ctx);
	nvgRoundedRect(ctx, sliderRect.x, sliderRect.y, sliderRect.width, sliderRect.height, radius);
	nvgFillColor(ctx, nvgRGBAf(0.0f, 0.0f, 0.0f, 0.6f));
	nvgFill(ctx);

	// value
	float nvalue = (value - min) / (max - min);
	float w = (sliderRect.width - 2) * nvalue;
	float h = w <= radius * 2.0f ? w : sliderRect.height - 2;
	nvgBeginPath(ctx);
	nvgRoundedRect(ctx, sliderRect.x + 1, b.height / 2 - h / 2, w, h, radius);
	nvgFillColor(ctx, nvgRGBf(1.0f, 1.0f, 1.0f));
	nvgFill(ctx);

	// arrows
	drawArrow(ctx, radius, b.height / 2, 4.0f, -1.0f, 0.5f, 0.5f, 0.5f, 0.7f);
	drawArrow(ctx, b.width - radius, b.height / 2, 4.0f, 1.0f, 0.5f, 0.5f, 0.5f, 0.7f);
}

void Slider::onPostDraw(NVGcontext* ctx, float deltaTime) {
	float animValue = m_anim.value(Curves::easeInOutBack, deltaTime);
	if (animValue <= 1e-5f) return;

	Rect b = bounds;
	Rect sliderRect = { 0, b.height / 2 - sliderHeight / 2, b.width, sliderHeight };

	float nvalue = (value - min) / (max - min);
	float popupX = (sliderRect.width - 2) * nvalue + 1;
	float popupY = 0;

	std::string valueText = std::vformat(valueFormat, std::make_format_args(value));

	nvgFontSize(ctx, 12.0f);

	float textBounds[4];
	nvgTextBounds(ctx, 0, 0, valueText.c_str(), nullptr, textBounds);

	float textW = textBounds[2] - textBounds[0];
	float textH = textBounds[3] - textBounds[1];

	float popupW = textW + 8;
	float popupH = textH + 6;

	popupY -= popupH;

	// offset for arrow
	popupY -= 5;

	float textX = popupX - textW / 2;
	float textY = popupY + popupH / 2;

	//std::cout << popupX << ", " << popupY << "\n";

	nvgSave(ctx);

	// animate offset
	nvgTranslate(ctx, 0.0f, std::lerp(10.0f, 0.0f, animValue));

	nvgBeginPath(ctx);
	nvgRoundedRect(ctx, popupX - popupW / 2, popupY, popupW, popupH, 6.0f);
	nvgMoveTo(ctx, popupX - 4, popupY + popupH);
	nvgLineTo(ctx, popupX + 4, popupY + popupH);
	nvgLineTo(ctx, popupX, popupY + popupH + 3);
	
	nvgFillColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, animValue));
	nvgFill(ctx);

	nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
	nvgFillColor(ctx, nvgRGBAf(0.0f, 0.0f, 0.0f, animValue));
	nvgTextBox(ctx, textX, textY, textW, valueText.c_str(), nullptr);
	nvgRestore(ctx);
}

void Slider::onMouseDown(int button, int x, int y) {
	if (button == 1) {
		calculateValue(x);
		m_dragging = true;

		m_anim.target(1.0f, 0.3f);
	}
}

void Slider::onMouseUp(int button, int x, int y) {
	m_dragging = false;
	m_anim.target(0.0f, 0.3f);
}

void Slider::onMouseMove(int x, int y, int dx, int dy) {
	if (m_dragging) {
		calculateValue(x);
	}
}

void Slider::onMouseLeave() {
	if (m_dragging) {
		m_dragging = false;
		m_anim.target(0.0f, 0.3f);
	}
}

void Slider::calculateValue(int mx) {
	Rect b = bounds;
	float normX = float(mx) / b.width;
	value = normX * (max - min) + min;

	float newStep = ::roundf(value / step);
	float newValue = newStep * step;

	if (newValue != value) {
		value = newValue;
		value = std::clamp(value, min, max);
		if (onChange) onChange(value);
	}
}
