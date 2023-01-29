#include "Control.h"

#include "GUISystem.h"
#include <iostream>

ControlID Control::g_ControlID = 0xA;

void Control::onDraw(NVGcontext* ctx, float deltaTime) {
	/*Rect b = bounds;
	nvgBeginPath(ctx);
	nvgRect(ctx, b.x-1, b.y-1, b.width+2, b.height+2);
	nvgStrokeColor(ctx, nvgRGB(0, 255, 255));
	nvgStroke(ctx);*/
}

bool Control::onEvent(WindowEvent ev) {
	std::vector<std::pair<ControlID, size_t>> orders;
	for (auto&& [cid, ctrl] : m_children) {
		orders.push_back({ cid, ctrl->m_order + m_order * 1000 });
	}

	bool consumed = false;
	for (const auto& [childId, _] : orders) {
		auto&& child = m_children[childId];
		consumed = child->onEvent(ev);
		if (consumed) {
			//std::cout << child->id() << " consumed the event.\n";
			break;
		}
	}
	if (consumed) return true;

	switch (ev.type) {
		case WindowEvent::mouseButton: return handleMouseButton(ev);
		case WindowEvent::mouseMotion: return handleMouseMotion(ev);
		case WindowEvent::keyboardKey: return handleKeyEvent(ev);
		case WindowEvent::textInput: return handleTextInput(ev);
	}

	return false;
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

ControlID Control::addChild(Control* control) {
	control->m_id = g_ControlID++;
	control->m_parent = this;
	m_toAdd.push_back(control);
	return control->m_id;
}

void Control::removeChild(ControlID control) {
	auto pos = m_children.find(control);
	if (pos == m_children.end()) {
		for (auto&& [cid, child] : m_children) {
			child->removeChild(control);
		}
		return;
	}
	
	m_toRemove.push_back(control);
}

void Control::flush() {
	for (auto ctrlId : m_toRemove) {
		auto&& ctrl = m_children[ctrlId];
		ctrl->parent(nullptr);
		m_children.erase(ctrlId);
	}
	for (auto ctrl : m_toAdd) {
		m_children[ctrl->m_id] = std::unique_ptr<Control>(ctrl);
	}

	m_toAdd.clear();
	m_toRemove.clear();

	for (const auto& [_, child] : m_children) {
		child->flush();
	}
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

Control* Control::child(ControlID id) {
	auto pos = m_children.find(id);
	if (pos == m_children.end()) {
		for (const auto& [cid, child] : m_children) {
			auto ctrl = child->child(id);
			if (ctrl != nullptr) {
				return ctrl;
			}
		}
		return nullptr;
	}
	return pos->second.get();
}

bool Control::handleMouseButton(WindowEvent ev) {
	Point screenPos = { ev.screenX, ev.screenY };
	Point mpos = screenToLocalPoint(screenPos);
	if (!localBounds().hasPoint(mpos)) return false;

	// this prevents the event from getting fired if the child (this) goes out of parent bounds
	if (m_parent && !m_parent->screenSpaceBounds().hasPoint(screenPos)) return false;

	if (ev.buttonState == WindowEvent::down) {
		requestFocus();
		onMouseDown(ev.button, mpos.x, mpos.y);
		m_dragging = true;
		return true;
	}
	else {
		onMouseUp(ev.button, mpos.x, mpos.y);
		m_dragging = false;
		return true;
	}
	return false;
}

bool Control::handleMouseMotion(WindowEvent ev) {
	Point screenPos = { ev.screenX, ev.screenY };
	Point mpos = screenToLocalPoint(screenPos);
	if (!localBounds().hasPoint(mpos) && !m_dragging) {
		checkMouseInside();
		return false;
	}

	if (m_parent && !m_parent->screenSpaceBounds().hasPoint(screenPos)) {
		checkMouseInside();
		return false;
	}

	if (!m_mouseInside) {
		onMouseEnter();
		m_mouseInside = true;
	}

	if (m_dragging) {
		onMouseDrag(mpos.x, mpos.y, ev.deltaX, ev.deltaY);
	}
	onMouseMove(mpos.x, mpos.y, ev.deltaX, ev.deltaY);

	return true;
}

bool Control::handleKeyEvent(WindowEvent ev) {
	if (ev.buttonState == WindowEvent::down) {
		return onKeyPress(ev.keyCode);
	}
	else {
		return onKeyRelease(ev.keyCode);
	}
}

bool Control::handleTextInput(WindowEvent ev) {
	return onType(ev.keyChar);
}

Control* Control::withFocusRequest() {
	if (m_focusRequested) {
		return this;
	}
	else {
		for (const auto& [cid, ctrl] : m_children) {
			auto ctrlPtr = ctrl->withFocusRequest();
			if (ctrlPtr) return ctrlPtr;
		}
	}
	return nullptr;
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

