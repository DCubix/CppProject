#include "Control.h"

#include <iostream>

void Control::onDraw(NVGcontext* ctx, float deltaTime) {
	/*Rect b = bounds;
	nvgBeginPath(ctx);
	nvgRect(ctx, b.x, b.y, b.width, b.height);
	nvgStrokeColor(ctx, nvgRGB(0, 255, 255));
	nvgStroke(ctx);*/
}

void Control::onEventReceived(const KeyboardEvent& e) {
	if (e.keyCode == 0) {
		onType(e.keyChar);
	}
	else {
		if (e.state == ButtonState::pressed) {
			onKeyPress(e.keyCode);
		}
		else {
			onKeyRelease(e.keyCode);
		}
	}
}

void Control::onEventReceived(const MouseMotionEvent& e) {
	Point mpos = screenToLocalPoint(e.screenPosition);
	if (!localBounds().hasPoint(mpos)) {
		checkMouseInside();
		return;
	}
	if (m_parent && !m_parent->screenSpaceBounds().hasPoint(e.screenPosition)) {
		checkMouseInside();
		return;
	}

	if (!m_mouseInside) {
		onMouseEnter();
		m_mouseInside = true;
	}

	onMouseMove(mpos.x, mpos.y, e.deltaX, e.deltaY);
}

void Control::onEventReceived(const MouseButtonEvent& e) {
	Point mpos = screenToLocalPoint(e.screenPosition);
	if (!localBounds().hasPoint(mpos)) return;

	// this prevents the event from getting fired if the child (this) goes out of parent bounds
	if (m_parent && !m_parent->screenSpaceBounds().hasPoint(e.screenPosition)) return;

	if (e.state == ButtonState::pressed) {
		requestFocus();
		onMouseDown(e.button, mpos.x, mpos.y);
	}
	else {
		onMouseUp(e.button, mpos.x, mpos.y);
	}
}

Rect Control::screenSpaceBounds() {
	Rect rec = bounds;
	if (m_parent) {
		Rect prec = m_parent->screenSpaceBounds();
		rec.x += prec.x;
		rec.y += prec.y;
	}
	return rec;
}

Rect Control::localBounds() {
	return { 0, 0, bounds.width, bounds.height };
}

Point Control::screenToLocalPoint(Point src) {
	Rect globBounds = screenSpaceBounds();
	Point pt = src;
	pt.x -= globBounds.x;
	pt.y -= globBounds.y;
	return pt;
}

void Control::checkMouseInside() {
	if (m_mouseInside) {
		onMouseLeave();
		m_mouseInside = false;
	}
}

bool Rect::hasPoint(Point point) {
	return point.x >= x &&
		   point.x <= x + width &&
		   point.y >= y &&
		   point.y <= y + height;
}

Rect& Rect::inflate(int amount) {
	x -= amount;
	y -= amount;
	width += amount * 2;
	height += amount * 2;
	return *this;
}

int Rect::distanceToPointSquared(Point p) {
		
		int insidePointX = std::clamp(p.x, x, x + width);
		int insidePointY = std::clamp(p.x, y, y + height);
		
		int diffX = p.x - insidePointX;
		int diffY = p.y - insidePointY;
		
		return diffX*diffX + diffY*diffY;

	}

SlicedRect SlicedRect::cutLeft(int size) {
	int minX = minx;
	minx = std::min(maxx, minx + size);
	return { minX, miny, minx, maxy };
}

SlicedRect SlicedRect::cutRight(int size) {
	int maxX = maxx;
	maxx = std::max(minx, maxx - size);
	return { maxx, miny, maxX, maxy };
}

SlicedRect SlicedRect::cutTop(int size) {
	int minY = miny;
	miny = std::min(maxy, miny + size);
	return { minx, minY, maxx, miny };
}

SlicedRect SlicedRect::cutBottom(int size) {
	int maxY = maxy;
	maxy = std::max(miny, maxy - size);
	return { minx, maxy, maxx, maxY };
}

Rect SlicedRect::toRect() {
	return { minx, miny, maxx - minx, maxy - miny };
}

Rect SlicedRect::toRectRelative() {
	return { 0, 0, maxx - minx, maxy - miny };
}

