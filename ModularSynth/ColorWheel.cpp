#include "ColorWheel.h"

#include <format>

constexpr float hueBarWidth = 18.0f;

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

	// Center triangle
	float r = r0 - 6;
	float ax = ::cosf(120.0f / 180.0f * NVG_PI) * r;
	float ay = ::sinf(120.0f / 180.0f * NVG_PI) * r;
	float bx = ::cosf(-120.0f / 180.0f * NVG_PI) * r;
	float by = ::sinf(-120.0f / 180.0f * NVG_PI) * r;

	m_triangle = {
		bx, by,
		ax, ay,
		r, 0.0f
	};

	float xform[9];
	nvgTransformRotate(xform, hue * NVG_PI * 2);

	for (size_t i = 0; i < 6; i += 2) {
		nvgTransformPoint(&m_triangle[i], &m_triangle[i + 1], xform, m_triangle[i], m_triangle[i + 1]);
		m_triangle[i] += cx;
		m_triangle[i + 1] += cy;
	}

	nvgBeginPath(ctx);
	nvgMoveTo(ctx, r, 0);
	nvgLineTo(ctx, ax, ay);
	nvgLineTo(ctx, bx, by);
	nvgClosePath(ctx);
	paint = nvgLinearGradient(ctx, r, 0, ax, ay, nvgHSLA(hue, 1.0f, 0.5f, 255), nvgRGBA(255, 255, 255, 255));
	nvgFillPaint(ctx, paint);
	nvgFill(ctx);
	paint = nvgLinearGradient(ctx, (r + ax) * 0.5f, (0 + ay) * 0.5f, bx, by, nvgRGBA(0, 0, 0, 0), nvgRGBA(0, 0, 0, 255));
	nvgFillPaint(ctx, paint);
	nvgFill(ctx);
	nvgStrokeColor(ctx, nvgRGBA(0, 0, 0, 64));
	nvgStroke(ctx);

	paint = nvgRadialGradient(ctx, ax, ay, 7, 9, nvgRGBA(0, 0, 0, 64), nvgRGBA(0, 0, 0, 0));
	nvgBeginPath(ctx);
	nvgRect(ctx, ax - 20, ay - 20, 40, 40);
	nvgCircle(ctx, ax, ay, 7);
	nvgPathWinding(ctx, NVG_HOLE);
	nvgFillPaint(ctx, paint);
	nvgFill(ctx);

	nvgRestore(ctx);

	// Select circle on triangle
	{
		Point A{ m_triangle[0], m_triangle[1] };
		Point B{ m_triangle[2], m_triangle[3] };
		Point C{ m_triangle[4], m_triangle[5] };

		m_triangleSelector = B*value + C*saturation + A*m_w;
	}

	nvgStrokeWidth(ctx, 4.0f);
	nvgBeginPath(ctx);
	nvgCircle(ctx, m_triangleSelector.x, m_triangleSelector.y, 5);
	nvgStrokeColor(ctx, nvgRGBA(255, 255, 0, 192));
	nvgStroke(ctx);


	nvgFillColor(ctx, nvgRGB(255, 255, 0));
	nvgText(ctx, 0.0f, 0.0f, std::format("u: {:.2f}, v: {:.2f}, w: {:.2f}", value, saturation, m_w).c_str(), nullptr);
	nvgText(ctx, 0.0f, 18.0f, std::format("x: {:.2f}, y: {:.2f}", m_triangleSelector.x, m_triangleSelector.y).c_str(), nullptr);

	nvgText(ctx, m_triangle[0], m_triangle[1], "A", nullptr);
	nvgText(ctx, m_triangle[2], m_triangle[3], "B", nullptr);
	nvgText(ctx, m_triangle[4], m_triangle[5], "C", nullptr);

	nvgRestore(ctx);
}

void ColorWheel::onMouseMove(int x, int y, int dx, int dy) {
	switch (state) {
		case DragState::hueRing: updateHue(x, y); break;
		case DragState::satValTriangle: updateSatVal(x, y); break;
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

	auto [u, v, w] = barycentric(
		mouse,
		{ m_triangle[0], m_triangle[1] },
		{ m_triangle[2], m_triangle[3] },
		{ m_triangle[4], m_triangle[5] }
	);
	bool inTriangle = u > 0.0f && v > 0.0f && w < 1.0f;

	if (inTriangle) {
		state = DragState::satValTriangle;
		updateSatVal(x, y);
	}
	else if (dist >= r0 && dist <= r1) {
		state = DragState::hueRing;
		updateHue(x, y);
	}
}

void ColorWheel::onMouseUp(int button, int x, int y) {
	state = DragState::idling;
}

void ColorWheel::updateHue(int x, int y) {
	Rect b = bounds;
	Point center{ b.width * 0.5f, b.height * 0.5f };
	Point mouse{ float(x), float(y) };
	Point cm = (mouse - center);

	float angle = ::atan2(cm.y, cm.x) + NVG_PI;
	hue = (angle / NVG_PI) * 0.5f + 0.5f;
}

void ColorWheel::updateSatVal(int x, int y) {
	Point mouse{ float(x), float(y) };
	auto [u, v, w] = barycentric(
		mouse,
		{ m_triangle[0], m_triangle[1] },
		{ m_triangle[2], m_triangle[3] },
		{ m_triangle[4], m_triangle[5] }
	);

	value = u;// std::clamp(u, 0.0f, 1.0f);
	saturation = v;//std::clamp(v, 0.0f, 1.0f);
	m_w = w;
}