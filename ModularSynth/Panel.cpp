#include "Panel.h"

#include "GUISystem.h"

constexpr float titleHeight = 38.0f;

void Panel::onDraw(NVGcontext* ctx, float deltaTime) {
	// ordering
	m_orders.clear();
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
	}

	nvgScissor(ctx, dbounds.x, dbounds.y, dbounds.width, dbounds.height);

	if (onCustomPaint) {
		nvgSave(ctx);
		onCustomPaint(ctx);
		nvgRestore(ctx);
	}

	int index = 0;

	if (m_layout) m_layout->beginLayout();
	for (auto&& [ childId, order ] : m_orders) {
		auto&& child = m_children[childId];

		if (m_layout) {
			m_layout->performLayout(
				child.get(),
				{ b.width, b.height - int(m_drawBackground ? titleHeight : 0) },
				index
			);
		}
		
		if (m_drawBackground && m_layout) {
			child->bounds.y += titleHeight;
		}

		nvgSave(ctx);
		Rect cbounds = child->bounds;
		nvgTranslate(ctx, cbounds.x, cbounds.y);

		child->onDraw(ctx, deltaTime);
		nvgRestore(ctx);

		nvgFillColor(ctx, nvgRGB(0, 255, 255));
		nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
		nvgText(ctx, cbounds.x, cbounds.y, std::to_string(child->id()).c_str(), nullptr);

		index++;
	}
	if (m_layout) m_layout->endLayout();

	/*nvgResetScissor(ctx);


	nvgBeginPath(ctx);
	nvgRect(ctx, 0, 0, b.width, b.height);
	nvgStrokeColor(ctx, nvgRGB(0, 255, 255));
	nvgStroke(ctx);*/
}

void Panel::onPostDraw(NVGcontext* ctx, float deltaTime) {
	for (auto&& [childId, order] : m_orders) {
		auto&& child = m_children[childId];

		nvgSave(ctx);

		// make local coordinates
		Rect bounds = child->screenSpaceBounds();
		nvgTranslate(ctx, bounds.x, bounds.y);

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

void ColumnLayout::beginLayout() {
	m_ypos = 0;
}

void ColumnLayout::performLayout(Control* control, Dimension parentSize, size_t index) {
	control->bounds.width = parentSize.width - padding * 2;
	control->bounds.x = padding;
	control->bounds.y = padding + m_ypos;
	m_ypos += control->bounds.height + gap;
}

void RowLayout::beginLayout() {
	m_xpos = 0;
}

void RowLayout::performLayout(Control* control, Dimension parentSize, size_t index) {
	const int gapTotal = (columns - 1) * gap;
	const int columnWidth = (parentSize.width - gapTotal - padding * 2) / columns;

	float expand = 1.0f;
	if (expansion.find(index) != expansion.end()) {
		expand = expansion[index];
	}

	control->bounds.height = parentSize.height - padding * 2;
	control->bounds.width = int(float(columnWidth) * expand);
	control->bounds.x = m_xpos + padding;
	control->bounds.y = padding;

	m_xpos += control->bounds.width + gap;
}

void ColumnFlowLayout::beginLayout() {
	m_xpos = 0;
	m_ypos = 0;
}

void ColumnFlowLayout::performLayout(Control* control, Dimension parentSize, size_t index) {
	const int gapTotal = (columns - 1) * gap;
	const int actualParentWidth = parentSize.width - gapTotal - padding * 2;
	const int columnWidth = actualParentWidth / columns;

	control->bounds.height = controlHeight;
	control->bounds.width = columnWidth;
	control->bounds.x = m_xpos + padding;
	control->bounds.y = m_ypos + padding;

	int next = control->bounds.width + gap;
	if (m_xpos + next >= actualParentWidth) {
		m_xpos = 0;
		m_ypos += controlHeight + gap;
	}
	else {
		m_xpos += next;
	}
}
