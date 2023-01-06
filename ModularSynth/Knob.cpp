#include "Knob.h"

#undef min
#undef max
#include <algorithm>
#include <iostream>
#include <format>

#define PI 3.141592654f

constexpr float thickness = 15.0f;
constexpr float rotation = PI / 2;
constexpr float gap = PI / 4.5f;

void Knob::onDraw(NVGcontext* ctx, float deltaTime) {
	Rect b = bounds;
	float radius = std::min(b.width, b.height) / 2;

	const float angleFrom = rotation - gap;
	const float angleSpan = (PI * 2.0f) - (gap * 2.0f);

	float nvalue = (value - min) / (max - min);

	nvgBeginPath(ctx);
	nvgArc(ctx, b.width / 2, b.height / 2, radius, angleFrom + gap * 2, angleFrom, NVG_CW);
	nvgArc(ctx, b.width / 2, b.height / 2, radius - thickness, angleFrom, angleFrom + gap * 2, NVG_CCW);

	nvgFillColor(ctx, nvgRGBAf(0.0f, 0.0f, 0.0f, 0.6f));
	nvgFill(ctx);

	nvgBeginPath(ctx);
	nvgArc(ctx, b.width / 2, b.height / 2, radius - 1, angleFrom + gap * 2, angleFrom + gap * 2 + angleSpan * nvalue, NVG_CW);
	nvgArc(ctx, b.width / 2, b.height / 2, radius + 1 - thickness, angleFrom + gap * 2 + angleSpan * nvalue, angleFrom + gap * 2, NVG_CCW);

	nvgFillColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
	nvgFill(ctx);

	std::string valueText = std::vformat(valueFormat, std::make_format_args(value));

	nvgFontSize(ctx, 12.0f);
	nvgFillColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
	nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
	nvgText(ctx, b.width / 2, b.height / 2, valueText.c_str(), nullptr);
}

void Knob::onMouseDown(int button, int x, int y) {
	if (button == 1) {
		m_dragging = true;
		calculateValue(x, y);
	}
}

void Knob::onMouseUp(int button, int x, int y) {
	m_dragging = false;
}

static float constrainAngle(float x) {
	x = ::fmodf(x + PI, PI * 2.0f);
	if (x < 0.0f) x += PI * 2.0f;
	return x - PI;
}

static float normalizeAngle(float x) {
	return ((constrainAngle(x) / PI) * 0.5f + 0.5f) * PI * 2.0f;
}

void Knob::onMouseMove(int x, int y, int dx, int dy) {
	if (m_dragging) {
		calculateValue(x, y);
	}
}

void Knob::onMouseLeave() {
	m_dragging = false;
}

void Knob::calculateValue(int x, int y) {
	Rect b = bounds;
	float ax = float(x) - b.width / 2.0f;
	float ay = float(y) - b.height / 2.0f;
	float angle = normalizeAngle((::atan2f(ay, ax) + PI) - rotation - gap);
	float span = (PI * 2.0f) - gap * 2.0f;

	value = (angle / span) * (max - min) + min;

	float newStep = ::roundf(value / step);
	float newValue = newStep * step;
	newValue = ::fmaxf(::fminf(newValue, max), min);

	if (newValue != value) {
		value = newValue;
		if (onChange) onChange(value);
	}
}
