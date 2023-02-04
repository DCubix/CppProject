#pragma once

#include "Control.h"
#include <memory>
#include <string>
#include <map>
#include <functional>

class Layout {
public:
	virtual void beginLayout() {}
	virtual void endLayout() {}
	virtual void performLayout(Control* control, Dimension parentSize, size_t index) = 0;
};

class ScrollBar;
class Panel : public Control {
public:

	Panel();

	void onDraw(NVGcontext* ctx, float deltaTime) override;
	void onPostDraw(NVGcontext* ctx, float deltaTime) override;

	void onMouseDown(int button, int x, int y) override;
	void onMouseUp(int button, int x, int y) override;
	void onMouseMove(int x, int y, int dx, int dy) override;
	void onMouseLeave() override;

	bool onEvent(WindowEvent ev) override;

	void setLayout(Layout* layout);

	void drawBackground(bool visible) { m_drawBackground = visible; }
	void draggable(bool draggable) { m_draggable = draggable; }

	std::string title{ "Panel" };

	std::function<void(NVGcontext*)> onCustomPaint;

protected:

	Point screenToLocalPoint(Point p) final;

private:
	std::vector<std::pair<ControlID, size_t>> m_orders;

	std::unique_ptr<Layout> m_layout;

	bool m_drawBackground{ true }, m_draggable{ false };
	bool m_dragging{ false };

	std::array<std::unique_ptr<ScrollBar>, 2> m_scrollBars;

};

class ColumnLayout : public Layout {
public:
	ColumnLayout(int padding = 6, int gap = 6) : padding(padding), gap(gap) {}

	void beginLayout() override;
	void performLayout(Control* control, Dimension parentSize, size_t index);

	int padding{ 6 }, gap{ 6 };
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

class ColumnFlowLayout : public Layout {
public:
	ColumnFlowLayout(
		int columns = 2,
		int padding = 6,
		int gap = 6,
		int controlHeight = 24
	) : controlHeight(controlHeight),
		columns(columns),
		padding(padding),
		gap(gap)
	{}

	void beginLayout() override;
	void performLayout(Control* control, Dimension parentSize, size_t index);

	int padding{ 6 }, columns{ 2 }, gap{ 6 }, controlHeight{ 24 };
private:
	int m_xpos{ 0 }, m_ypos{ 0 };
};
