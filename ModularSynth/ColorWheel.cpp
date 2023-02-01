#include "ColorWheel.h"

#include <format>

constexpr float hueBarWidth = 16.0f;

static void rgbToHSV(float r, float g, float b, float& h, float& s, float& v) {
	float min, max, delta;

	min = std::min(std::min(r, g), b);
	max = std::max(std::max(r, g), b);

	v = max;
	delta = max - min;

	if (delta < 1e-5f) {
		s = 0.0f;
		h = 0.0f;
		return;
	}

	if (max > 0.0f) {
		s = (delta / max);
	}
	else {
		s = 0.0f;
		h = NAN;
		return;
	}

	if (r >= max) {
		h = (g - b) / delta;
	}
	else {
		if (g >= max) {
			h = 2.0f + (b - r) / delta;
		}
		else {
			h = 4.0f + (r - g) / delta;
		}
	}

	h *= 60.0;

	if (h < 0.0) {
		h += 360.0;
	}

	h /= 360.0f;
}

static void hsvToRGB(float h, float s, float v, float& r, float& g, float& b) {
	int i = int(h * 6);
	float f = h * 6 - i;
	float p = v * (1 - s);
	float q = v * (1 - f * s);
	float t = v * (1 - (1 - f) * s);

	switch (i % 6) {
		case 0: r = v, g = t, b = p; break;
		case 1: r = q, g = v, b = p; break;
		case 2: r = p, g = v, b = t; break;
		case 3: r = p, g = q, b = v; break;
		case 4: r = t, g = p, b = v; break;
		case 5: r = v, g = p, b = q; break;
	}
}

static std::tuple<float, float, float> barycentric(Point p, Point a, Point b, Point c) {
	float u = (a.x * (c.y - a.y) + (p.y - a.y) * (c.x - a.x) - p.x * (c.y - a.y)) / ((b.y - a.y) * (c.x - a.x) - (b.x - a.x) * (c.y - a.y));
	float v = (p.y - a.y - u * (b.y - a.y)) / (c.y - a.y);

	return {
		u,
		v,
		1.0f - u - v
	};
}

void ColorWheel::onDraw(NVGcontext* ctx, float deltaTime) {
	Rect b = bounds;

	nvgSave(ctx);

	float hue = m_hsv[0], saturation = m_hsv[1], value = m_hsv[2];

	float cx = b.width * 0.5f;
	float cy = b.height * 0.5f;
	float r1 = (b.width < b.height ? b.width : b.height) * 0.5f - 5.0f;
	float r0 = r1 - hueBarWidth;
	float aeps = 0.5f / r1;

	for (int i = 0; i < 6; i++) {
		float a0 = (float)i / 6.0f * NVG_PI * 2.0f - aeps;
		float a1 = (float)(i + 1.0f) / 6.0f * NVG_PI * 2.0f + aeps;
		nvgBeginPath(ctx);
		nvgArc(ctx, cx, cy, r0, a0, a1, NVG_CW);
		nvgArc(ctx, cx, cy, r1, a1, a0, NVG_CCW);
		nvgClosePath(ctx);

		float ax = cx + ::cosf(a0) * (r0 + r1) * 0.5f;
		float ay = cy + ::sinf(a0) * (r0 + r1) * 0.5f;
		float bx = cx + ::cosf(a1) * (r0 + r1) * 0.5f;
		float by = cy + ::sinf(a1) * (r0 + r1) * 0.5f;

		NVGpaint paint = nvgLinearGradient(ctx, ax, ay, bx, by, nvgHSLA(a0 / (NVG_PI * 2), 1.0f, 0.55f, 255), nvgHSLA(a1 / (NVG_PI * 2), 1.0f, 0.55f, 255));
		nvgFillPaint(ctx, paint);
		nvgFill(ctx);
	}

	nvgBeginPath(ctx);
	nvgCircle(ctx, cx, cy, r0 - 0.5f);
	nvgCircle(ctx, cx, cy, r1 + 0.5f);
	nvgStrokeColor(ctx, nvgRGBA(0, 0, 0, 64));
	nvgStrokeWidth(ctx, 1.0f);
	nvgStroke(ctx);

	// Selector
	nvgSave(ctx);
	nvgTranslate(ctx, cx, cy);
	nvgRotate(ctx, hue * NVG_PI * 2);

	// Marker on
	nvgStrokeWidth(ctx, 2.0f);
	nvgBeginPath(ctx);
	nvgRect(ctx, r0 - 1, -3, r1 - r0 + 2, 6);
	nvgStrokeColor(ctx, nvgRGBA(255, 255, 255, 192));
	nvgStroke(ctx);

	NVGpaint paint = nvgBoxGradient(ctx, r0 - 3, -5, r1 - r0 + 6, 10, 2, 4, nvgRGBA(0, 0, 0, 128), nvgRGBA(0, 0, 0, 0));
	nvgBeginPath(ctx);
	nvgRect(ctx, r0 - 2 - 10, -4 - 10, r1 - r0 + 4 + 20, 8 + 20);
	nvgRect(ctx, r0 - 2, -4, r1 - r0 + 4, 8);
	nvgPathWinding(ctx, NVG_HOLE);
	nvgFillPaint(ctx, paint);
	nvgFill(ctx);

	nvgRestore(ctx);

	nvgSave(ctx);
	nvgTranslate(ctx, cx, cy);
	// current color view
	nvgBeginPath(ctx);

	nvgMoveTo(ctx, -r1, r1);
	nvgLineTo(ctx, -r1 + 32.0f, r1);
	nvgLineTo(ctx, -r1, r1 - 32.0f);
	nvgClosePath(ctx);

	nvgFillColor(ctx, nvgRGBf(m_color.r, m_color.g, m_color.b));
	nvgFill(ctx);

	nvgStrokeColor(ctx, nvgRGBf(1.0f, 1.0f, 1.0f));
	nvgStroke(ctx);

	// Center square
	float r = (r1 - 1) / 2.0f;

	nvgBeginPath(ctx);
	nvgRect(ctx, -r, -r, r * 2, r * 2);

	paint = nvgLinearGradient(ctx, -r, 0.0f, r, 0.0f, nvgRGBA(255, 255, 255, 255), nvgHSLA(hue, 1.0f, 0.5f, 255));
	nvgFillPaint(ctx, paint);
	nvgFill(ctx);

	paint = nvgLinearGradient(ctx, 0.0f, -r, 0.0f, r, nvgRGBA(0, 0, 0, 0), nvgRGBA(0, 0, 0, 255));
	nvgFillPaint(ctx, paint);
	nvgFill(ctx);

	nvgStrokeColor(ctx, nvgRGBA(0, 0, 0, 64));
	nvgStroke(ctx);
	//
	nvgRestore(ctx);

	nvgStrokeWidth(ctx, 2.0f);
	nvgBeginPath(ctx);
	nvgCircle(ctx, m_selector.x, m_selector.y, 4);
	nvgStrokeColor(ctx, nvgRGBA(255, 255, 255, 192));
	nvgStroke(ctx);

	//nvgFillColor(ctx, nvgRGB(255, 255, 0));
	//nvgText(ctx, 0.0f, 0.0f, std::format("state: {}", size_t(state)).c_str(), nullptr);
	//nvgText(ctx, 0.0f, 6.0f, std::format("val: {:.2f}, sat: {:.2f}, w: {:.2f}", value, saturation, m_w).c_str(), nullptr);
	//nvgText(ctx, 0.0f, 24.0f, std::format("x: {:.2f}, y: {:.2f}", m_triangleSelector.x, m_triangleSelector.y).c_str(), nullptr);

	//nvgText(ctx, m_triangle[0], m_triangle[1], "A", nullptr);
	//nvgText(ctx, m_triangle[2], m_triangle[3], "B", nullptr);

	nvgRestore(ctx);

	/*nvgBeginPath(ctx);
	nvgRect(ctx, 0.0f, 0.0f, b.width, b.height);
	nvgStrokeColor(ctx, nvgRGB(0, 255, 255));
	nvgStroke(ctx);*/
}

void ColorWheel::onMouseMove(int x, int y, int dx, int dy) {
	switch (state) {
		case DragState::hueRing: updateHue(x, y); break;
		case DragState::satVal: updateSatVal(x, y); break;
		default: break;
	}
}

void ColorWheel::onMouseDown(int button, int x, int y) {
	if (button != 1) return;

	Rect b = bounds;
	float r1 = (b.width < b.height ? b.width : b.height) * 0.5f - 5.0f;
	float r0 = r1 - hueBarWidth;

	Point center{ b.width * 0.5f, b.height * 0.5f };
	Point mouse{ float(x), float(y) };
	Point cm = (mouse - center);
	float dist = ::sqrtf(cm.dot(cm));

	float cx = b.width * 0.5f;
	float cy = b.height * 0.5f;
	float r = (r1 - 1) / 2.0f;

	Rect square{
		cx - r,
		cy - r,
		r * 2,
		r * 2
	};

	if (square.hasPoint(mouse)) {
		state = DragState::satVal;
		onMouseMove(x, y, 0, 0);
	}
	else if (dist >= r0 && dist <= r1) {
		state = DragState::hueRing;
		onMouseMove(x, y, 0, 0);
	}
}

void ColorWheel::onMouseUp(int button, int x, int y) {
	state = DragState::idling;
}

void ColorWheel::onMouseLeave() {
	state = DragState::idling;
}

void ColorWheel::updateHue(int x, int y) {
	Rect b = bounds;
	Point center{ b.width * 0.5f, b.height * 0.5f };
	Point mouse{ float(x), float(y) };
	Point cm = (mouse - center);

	float angle = ::atan2(cm.y, cm.x) + NVG_PI;
	m_hsv[0] = (angle / NVG_PI) * 0.5f + 0.5f;

	updateColor();
	updatePoint();
}

void ColorWheel::updateSatVal(int x, int y) {
	Point mouse{ float(x), float(y) };
	Rect b = bounds;

	float cx = b.width * 0.5f;
	float cy = b.height * 0.5f;
	float r1 = (b.width < b.height ? b.width : b.height) * 0.5f - 5.0f;
	float r = (r1 - 1) / 2.0f;

	Rect square{
		cx - r,
		cy - r,
		r * 2,
		r * 2
	};

	m_hsv[1] = std::clamp((mouse.x - square.x) / square.width, 0.0f, 1.0f);
	m_hsv[2] = 1.0f - std::clamp((mouse.y - square.y) / square.height, 0.0f, 1.0f);

	updateColor();
	updatePoint();
}

void ColorWheel::updateColor() {
	hsvToRGB(m_hsv[0], m_hsv[1], m_hsv[2], m_color.r, m_color.g, m_color.b);
	if (onChange) onChange(m_color);
}

void ColorWheel::updatePoint() {
	Rect b = bounds;

	float cx = b.width * 0.5f;
	float cy = b.height * 0.5f;
	float r1 = (b.width < b.height ? b.width : b.height) * 0.5f - 5.0f;
	float r = (r1 - 1) / 2.0f;

	Rect square{
		cx - r,
		cy - r,
		r * 2,
		r * 2
	};

	m_selector.x = square.x + m_hsv[1] * square.width;
	m_selector.y = square.y + (1.0f - m_hsv[2]) * square.height;
}

void ColorWheel::color(Color col) {
	m_color = col;
	rgbToHSV(col.r, col.g, col.b, m_hsv[0], m_hsv[1], m_hsv[2]);
	updateColor();

	if (onChange) onChange(m_color);

	updatePoint();
}
