#pragma once

#include "Control.h"
#include <memory>
#include <string>
#include <map>

class Layout {
public:
	virtual void beginLayout() {}
	virtual void endLayout() {}
	virtual void performLayout(Control* control, Dimension parentSize, size_t index) = 0;
};

class Panel : public Control {
public:
	void onDraw(NVGcontext* ctx, float deltaTime) override;

	void setLayout(Layout* layout);
	void addChild(Control* control);
	void removeChild(Control* control);

	std::vector<Control*> onGetExtraControls() override { return m_children; }
	void clearExtraControls() override;

	std::vector<Control*> children() { return m_children; }

	void drawBackground(bool visible) { m_drawBackground = visible; }

	std::string title{ "Panel" };

private:
	std::vector<Control*> m_children;
	std::unique_ptr<Layout> m_layout;

	bool m_drawBackground{ true };
};

class ColumnLayout : public Layout {
public:
	ColumnLayout(int padding = 6) : padding(padding) {}

	void beginLayout() override;
	void performLayout(Control* control, Dimension parentSize, size_t index);

	int padding{ 6 };
private:
	int m_ypos{ 0 };
};

class RowLayout : public Layout {
public:
	RowLayout(int columns = 2, int padding = 6, int gap = 6) : columns(columns), padding(padding), gap(gap) {}

	void beginLayout() override;
	void performLayout(Control* control, Dimension parentSize, size_t index);

	int padding{ 6 }, columns{ 2 }, gap{ 6 };
	std::map<size_t, float> expansion;
private:
	int m_xpos{ 0 };
};

