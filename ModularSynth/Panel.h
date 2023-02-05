#pragma once

#include "Control.h"
#include <memory>
#include <string>
#include <map>
#include <functional>

class Layout {
public:
	virtual Dimension apply(const std::vector<Control*> targets, Dimension parentSize) = 0;
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

	bool onEvent(WindowEvent ev, Point offset) override;

	void setLayout(Layout* layout);

	void drawBackground(bool visible) { m_drawBackground = visible; }
	void draggable(bool draggable) { m_draggable = draggable; }

	std::string title{ "Panel" };

	std::function<void(NVGcontext*)> onCustomPaint;

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

	Dimension apply(const std::vector<Control*> targets, Dimension parentSize);

	int padding{ 6 }, gap{ 6 };

};

class RowLayout : public Layout {
public:
	RowLayout(int padding = 6, int gap = 6) : padding(padding), gap(gap) {}

	Dimension apply(const std::vector<Control*> targets, Dimension parentSize);

	int padding{ 6 }, gap{ 6 };
	std::map<size_t, float> expansion;

};
