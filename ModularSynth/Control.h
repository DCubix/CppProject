#pragma once

#include "Window.h"
#include "nanovg/nanovg.h"

#include <map>
#include <array>
#include <cmath>
#include <memory>

using ControlID = size_t;

enum class HorizontalAlignment {
	left = 0,
	center,
	right
};

struct Point { int x, y; };

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

class GUISystem;
class Control {
	friend class GUISystem;
	friend class Panel;
public:
	Control() = default;
	virtual ~Control() = default;

	virtual void onDraw(NVGcontext* ctx, float deltaTime);
	virtual void onPostDraw(NVGcontext* ctx, float deltaTime) {}
	
	void flush();

	Control* parent() { return m_parent; }
	void parent(Control* control) { m_parent = control; }

	virtual void onMouseDown(int button, int x, int y) {}
	virtual void onMouseUp(int button, int x, int y) {}
	virtual void onMouseMove(int x, int y, int dx, int dy) {}
	virtual void onMouseDrag(int x, int y, int dx, int dy) {}
	virtual void onMouseEnter() {}
	virtual void onMouseLeave() {}
	virtual void onFocus() {}
	virtual void onBlur() {}
	virtual void onKeyPress(int keyCode) {}
	virtual void onKeyRelease(int keyCode) {}
	virtual bool onType(TCHAR charCode) { return false; }

	virtual std::vector<Control*> onGetExtraControls() { return std::vector<Control*>(); }
	virtual void clearExtraControls() {}

	virtual bool onEvent(WindowEvent ev);

	Rect screenSpaceBounds();
	Rect localBounds();

	ControlID addChild(Control* control);
	void removeChild(ControlID control);
	Control* child(ControlID id);

	bool focused() const { return m_focused; }
	void requestFocus() { m_focusRequested = true; }

	void setOrder(size_t order) { m_order = order; }

	ControlID id() const { return m_id; }

	Rect bounds{};

	static ControlID g_ControlID;

protected:
	Control* m_parent{ nullptr };
	std::map<ControlID, std::unique_ptr<Control>> m_children{};

	std::vector<ControlID> m_toRemove;
	std::vector<Control*> m_toAdd;

	ControlID m_id{ 0 }, m_currentFocus{ 0 };

	size_t m_order{ 0 };

	bool m_mouseInside{ false },
		m_focusRequested{ false },
		m_focused{ false },
		m_dragging{ false };

	Point screenToLocalPoint(Point src);
	void checkMouseInside();

	bool handleMouseButton(WindowEvent ev);
	bool handleMouseMotion(WindowEvent ev);
	bool handleKeyEvent(WindowEvent ev);
	bool handleTextInput(WindowEvent ev);

	void checkFocus();
};

