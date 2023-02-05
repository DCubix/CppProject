#include "Panel.h"
#include "ScrollBar.h"
#include "GUISystem.h"

#include <iostream>

constexpr float titleHeight = 38.0f;

Panel::Panel() {

	for(size_t i = 0; i < m_scrollBars.size(); i++) {
		m_scrollBars[i] = std::make_unique<ScrollBar>();
		m_scrollBars[i]->orientation = SBOrientation(i);
	}

	m_scrollBars[1]->pageStep = 1.0f;
	m_scrollBars[1]->pageStep = 1.0f;

	m_scrollBars[0]->pageMax = 0.f;
	m_scrollBars[0]->pageMax = 0.f;

}

void Panel::onDraw(NVGcontext* ctx, float deltaTime) {
	// ordering
	m_orders.clear();
	for(auto& sb: m_scrollBars)
		sb->parent(this);


	m_scrollBars[0]->bounds.x = 0.f;
	m_scrollBars[0]->bounds.y = bounds.height - 20.f;
	m_scrollBars[0]->bounds.width = bounds.width - (m_scrollBars[1]->shouldShow() ? m_scrollBars[1]->bounds.width : 0.0f);
	m_scrollBars[0]->bounds.height = 20.f;
	
	m_scrollBars[1]->bounds.x = bounds.width - 20.f;
	m_scrollBars[1]->bounds.y = (title.empty() ? 0 : titleHeight);
	m_scrollBars[1]->bounds.width  = 20.f;
	m_scrollBars[1]->bounds.height = bounds.height - (m_scrollBars[0]->shouldShow() ? m_scrollBars[0]->bounds.height : 0.0f) - (title.empty() ? 0 : titleHeight);

	int rw = (m_scrollBars[1]->shouldShow() ? m_scrollBars[1]->bounds.width : 0.0f);
	int rh = (m_scrollBars[0]->shouldShow() ? m_scrollBars[0]->bounds.height : 0.0f);

	m_scrollBars[0]->pageSize = bounds.width - rw;
	m_scrollBars[1]->pageSize = bounds.height - rh;

	for (auto&& [cid, ctrl] : m_children) {
		m_orders.push_back({ cid, ctrl->m_order + m_order * 1000 });
	}

	std::sort(
		m_orders.begin(),
		m_orders.end(),
		[](const std::pair<ControlID, size_t>& a, const std::pair<ControlID, size_t>& b) {
			return a.second < b.second;
		}
	);

	Rect b = bounds;
	Rect dbounds = { 1, 1, b.width - 2, b.height - 2 };

	if (m_drawBackground) {
		nvgBeginPath(ctx);
		nvgRoundedRect(ctx, 0.0f, 0.0f, b.width, b.height, 16.0f);
		nvgFillColor(ctx, nvgRGBAf(0.0f, 0.0f, 0.0f, 0.35f));
		nvgFill(ctx);

		nvgBeginPath(ctx);
		nvgRoundedRectVarying(ctx, 0.0f, 0.0f, b.width, titleHeight, 16.0f, 16.0f, 0.0f, 0.0f);
		nvgFillColor(ctx, nvgRGBAf(0.0f, 0.0f, 0.0f, 0.45f));
		nvgFill(ctx);

		nvgFillColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
		nvgFontSize(ctx, 16.0f);
		nvgTextAlign(ctx, NVG_ALIGN_MIDDLE);
		nvgText(ctx, 16.0f, titleHeight / 2 + 1.5f, title.c_str(), nullptr);

		dbounds.y += titleHeight;
		dbounds.height -= titleHeight;
	}

	nvgSave(ctx);
	nvgIntersectScissor(ctx, dbounds.x, dbounds.y, dbounds.width - rw, dbounds.height - rh);

	if (onCustomPaint) {
		nvgSave(ctx);
		onCustomPaint(ctx);
		nvgRestore(ctx);
	}

	std::vector<Control*> children;
	for (auto&& [childId, order] : m_orders) {
		children.push_back(m_children[childId].get());
	}

	if (m_layout) {
		Dimension size = m_layout->apply(children, { int(b.width) - rw, int(b.height - (m_drawBackground ? titleHeight : 0)) - rh });

		m_scrollBars[0]->pageMax = size.width;
		m_scrollBars[1]->pageMax = size.height;
	}

	int index = 0;
	for (auto&& child : children) {
		if (m_drawBackground && m_layout) {
			child->bounds.y += titleHeight;
		}

		nvgSave(ctx);

		Rect cbounds = child->bounds;
		nvgTranslate(ctx, cbounds.x - m_scrollBars[0]->page, cbounds.y - m_scrollBars[1]->page);

		child->onDraw(ctx, deltaTime);
		nvgRestore(ctx);

		/*nvgFillColor(ctx, nvgRGB(0, 255, 255));
		nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
		nvgText(ctx, cbounds.x, cbounds.y, std::to_string(child->id()).c_str(), nullptr);*/

		index++;
	}

	nvgRestore(ctx);
	for(auto& sb: m_scrollBars) {
		nvgSave(ctx);
		nvgTranslate(ctx, sb->bounds.x, sb->bounds.y);
		if(sb->shouldShow()) 
			sb->onDraw(ctx, deltaTime);
		nvgRestore(ctx);
	}

	/*nvgStrokeColor(ctx, nvgRGB(0, 255, 255));
	nvgBeginPath(ctx);
	nvgRect(ctx, 0, 0, b.width, b.height);
	nvgStroke(ctx);*/

}

bool Panel::onEvent(WindowEvent ev, Point offset) {
	for(auto& sb: m_scrollBars) {
		if(sb->shouldShow()) {
			bool consumed = sb->onEvent(ev, offset);
			if(consumed) return true;
		}
	}

	return Control::onEvent(ev, { offset.x + m_scrollBars[0]->page, offset.y + m_scrollBars[1]->page });
}

void Panel::onPostDraw(NVGcontext* ctx, float deltaTime) {
	for (auto&& [childId, order] : m_orders) {
		auto&& child = m_children[childId];

		nvgSave(ctx);

		// make local coordinates
		Rect bounds = child->bounds;
		nvgTranslate(ctx, bounds.x - m_scrollBars[0]->page, bounds.y - m_scrollBars[1]->page);

		child->onPostDraw(ctx, deltaTime);
		nvgRestore(ctx);
	}

}

void Panel::setLayout(Layout* layout) {
	m_layout.reset(layout);
}

void Panel::onMouseDown(int button, int x, int y) {
	if (button == 1 && m_draggable && m_parent == 0) { // only drag top level panels
		m_dragging = true;
	}
}

void Panel::onMouseUp(int button, int x, int y) {
	m_dragging = false;
}

void Panel::onMouseMove(int x, int y, int dx, int dy) {
	if (m_dragging) {
		bounds.x += dx;
		bounds.y += dy;
	}
}

void Panel::onMouseLeave() {
	m_dragging = false;
}

Dimension ColumnLayout::apply(const std::vector<Control*> targets, Dimension parentSize) {
	float yPos = padding;

	Dimension size{ padding, padding };

	for (auto ctrl : targets) {
		ctrl->bounds.x = padding;
		ctrl->bounds.y = yPos;
		ctrl->bounds.width = parentSize.width - padding * 2;
		yPos += ctrl->bounds.height + gap;
	}

	size.width = parentSize.width - padding;
	size.height = (int(yPos) + padding) - gap;

	return size;
}

Dimension RowLayout::apply(const std::vector<Control*> targets, Dimension parentSize) {
	float xPos = padding;

	Dimension size{ padding, padding };

	size_t index = 0;
	float parentWidth = parentSize.width - padding * 2;
	for (auto ctrl : targets) {
		ctrl->bounds.x = xPos;
		ctrl->bounds.y = padding;

		auto exp = expansion.find(index);
		if (exp != expansion.end() && exp->second > 0.0f) {
			ctrl->bounds.width = parentWidth * exp->second;
		}

		ctrl->bounds.height = parentSize.height - padding * 2;
		xPos += ctrl->bounds.width + gap;

		parentWidth -= (ctrl->bounds.width + gap);
		if (parentWidth <= 0.0f) parentWidth = 0.0f;

		index++;
	}

	size.width = (int(xPos) + padding) - gap;
	size.height = parentSize.height + padding;

	return size;
}
