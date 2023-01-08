#pragma once

#include "EventSystem.h"
#include "nanovg/nanovg.h"

#include <array>

enum class HorizontalAlignment {
	left = 0,
	center,
	right
};

struct Rect {
	int x{ 0 }, y{ 0 }, width{100}, height{100};

	bool hasPoint(Point point);
	void inflate(int amount = 1);
};

struct Dimension {
	int width{ 100 }, height{ 100 };
};

struct Color {
	float r, g, b, a;

	float luminance() const { return 0.2126f * r + 0.7152f * g + 0.0722f * b; };
};

class Control : public MouseButtonListener,
				public MouseMotionListener,
				public KeyboardListener
{
public:
	Control() = default;
	virtual ~Control() = default;

	virtual void onDraw(NVGcontext* ctx, float deltaTime);
	virtual void onPostDraw(NVGcontext* ctx, float deltaTime) {}

	Control* parent() { return m_parent; }
	void parent(Control* control) { m_parent = control; }

	virtual void onMouseDown(int button, int x, int y) {}
	virtual void onMouseUp(int button, int x, int y) {}
	virtual void onMouseMove(int x, int y, int dx, int dy) {}
	virtual void onMouseEnter() {}
	virtual void onMouseLeave() {}
	virtual void onKeyPress(int keyCode) {}
	virtual void onKeyRelease(int keyCode) {}
	virtual void onType(TCHAR charCode) {}

	void onEventReceived(const KeyboardEvent& e);
	void onEventReceived(const MouseMotionEvent& e);
	void onEventReceived(const MouseButtonEvent& e);

	Rect screenSpaceBounds();
	Rect localBounds();

	Rect bounds{};
private:
	Control* m_parent{ nullptr };
	bool m_mouseInside{ false };

	Point screenToLocalPoint(Point src);
	void checkMouseInside();
};

