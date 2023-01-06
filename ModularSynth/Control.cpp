#include "Control.h"

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

void Rect::inflate(int amount) {
	x -= amount;
	y -= amount;
	width += amount * 2;
	height += amount * 2;
}
