#pragma once

#include "EventSystem.h"
#include "nanovg/nanovg.h"

#include <array>
#include <cmath>

using ControlID = size_t;

enum class HorizontalAlignment {
	left = 0,
	center,
	right
};

struct Rect {
	int x{ 0 }, y{ 0 }, width{ 100 }, height{ 100 };

	int distanceToPointSquared(Point p);
	bool hasPoint(Point point);
	Rect& inflate(int amount = 1);
};

struct SlicedRect {
	int minx{ 0 }, miny{ 0 }, maxx{ 100 }, maxy{ 100 };

	SlicedRect cutLeft(int size);
	SlicedRect cutRight(int size);
	SlicedRect cutTop(int size);
	SlicedRect cutBottom(int size);

	Rect toRect();
	Rect toRectRelative();
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
	friend class GUISystem;
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
	virtual void onFocus() {}
	virtual void onBlur() {}
	virtual void onKeyPress(int keyCode) {}
	virtual void onKeyRelease(int keyCode) {}
	virtual void onType(TCHAR charCode) {}

	virtual std::vector<Control*> onGetExtraControls() { return std::vector<Control*>(); }
	virtual void clearExtraControls() {}

	void onEventReceived(const KeyboardEvent& e);
	void onEventReceived(const MouseMotionEvent& e);
	void onEventReceived(const MouseButtonEvent& e);

	Rect screenSpaceBounds();
	Rect localBounds();

	bool focused() const { return m_focused; }
	void requestFocus() { m_focusRequested = true; }

	void setOrder(size_t order) { m_order = order; }

	ControlID id() const { return m_id; }

	Rect bounds{};


protected:
	Control* m_parent{ nullptr };
	ControlID m_id{ 0 };

	size_t m_order{ 0 };

	bool m_mouseInside{ false }, m_focusRequested{ false }, m_focused{ false };

	Point screenToLocalPoint(Point src);
	void checkMouseInside();
};

