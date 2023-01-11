#pragma once

#include "TextureNodes.hpp"
#include "NodeEditor.h"

#include "GUISystem.h"
#include "Slider.h"
#include "Panel.h"
#include "Label.h"

#define hex2rgbf(h) { float((h & 0xFF0000) >> 16) / 255.0f, float((h & 0xFF00) >> 8) / 255.0f, float(h & 0xFF) / 255.0f, 1.0f }

static constexpr Color generatorNodeColor = hex2rgbf(0x47b394);
static constexpr Color operatorNodeColor = hex2rgbf(0xb86335);

class VisualTextureNode : public VisualNode {
public:
	Dimension extraSize() override { return { 100, 100 }; }
	void onExtraDraw(NVGcontext* ctx, float deltaTime) override;

	int image = -1;
};

/* UI Editing */
using GuiBuilder = std::function<Control* (GUISystem*, VisualNode*)>;

/* ========== */

using NodeBuilder = std::function<VisualTextureNode* (NodeEditor*, const std::string&, const std::string&)>;

struct NodeContructor {
	std::string code, name;
	NodeBuilder onCreate;
	GuiBuilder onGui;
};

#define NodeCtor(T, c) [](NodeEditor* editor, const std::string& name, const std::string& code) { return editor->create<T, VisualTextureNode>(name, code, c); }

static Control* gui_ColorNode(GUISystem* gui, VisualNode* node) {
	Panel* pnl = new Panel();
	pnl->drawBackground(false);
	pnl->bounds = { 0, 0, 0, 0 };
	pnl->setLayout(new ColumnLayout());
	gui->addControl(pnl);

	const std::string labels[] = { "Red", "Geen", "Blue", "Alpha" };

	GraphicsNode* nd = (GraphicsNode*)node->node();
	for (size_t i = 0; i < 4; i++) {
		Slider* sld = new Slider();
		gui->addControl(sld);

		sld->value = nd->param("Color").value[i];
		sld->onChange = [=](float v) {
			nd->setParam("Color", i, v);
		};

		Label* lbl = new Label();
		lbl->text = labels[i];
		lbl->alignment = HorizontalAlignment::right;
		gui->addControl(lbl);

		Panel* row = new Panel();
		row->drawBackground(false);
		row->bounds = { 0, 0, 0, 30 };

		RowLayout* rl = new RowLayout(2, 3);
		rl->expansion[0] = 0.5f;
		rl->expansion[1] = 1.5f;

		row->setLayout(rl);

		gui->addControl(row);

		row->addChild(lbl);
		row->addChild(sld);

		pnl->addChild(row);

		pnl->bounds.height += 30;
	}

	return pnl;
}

static Control* gui_MixNode(GUISystem* gui, VisualNode* node) {
	Panel* pnl = new Panel();
	pnl->drawBackground(false);
	pnl->bounds = { 0, 0, 0, 60 };
	pnl->setLayout(new ColumnLayout());
	gui->addControl(pnl);

	GraphicsNode* nd = (GraphicsNode*)node->node();
	Slider* sld = new Slider();
	gui->addControl(sld);

	Label* lbl = new Label();
	lbl->text = "Factor";
	lbl->bounds = { 0, 0, 0, 18 };
	gui->addControl(lbl);

	sld->value = nd->param("Factor").value[0];
	sld->onChange = [=](float v) {
		nd->setParam("Factor", v);
	};
	sld->bounds = { 0, 0, 0, 30 };

	pnl->addChild(lbl);
	pnl->addChild(sld);

	return pnl;
}

static Control* gui_SimpleGradientNode(GUISystem* gui, VisualNode* node) {
	Panel* pnl = new Panel();
	pnl->drawBackground(false);
	pnl->bounds = { 0, 0, 0, 60 };
	pnl->setLayout(new ColumnLayout());
	gui->addControl(pnl);

	GraphicsNode* nd = (GraphicsNode*)node->node();
	Slider* sld = new Slider();
	gui->addControl(sld);

	Label* lbl = new Label();
	lbl->text = "Angle";
	lbl->bounds = { 0, 0, 0, 18 };
	gui->addControl(lbl);

	sld->value = nd->param("Angle").value[0];
	sld->onChange = [=](float v) {
		nd->setParam("Angle", v);
	};
	sld->min = -PI;
	sld->max = PI;
	sld->bounds = { 0, 0, 0, 30 };

	pnl->addChild(lbl);
	pnl->addChild(sld);

	return pnl;
}

static NodeContructor nodeTypes[] = {
	{ "COL", "Color", NodeCtor(ColorNode, generatorNodeColor), gui_ColorNode },
	{ "MIX", "Mix", NodeCtor(MixNode, operatorNodeColor), gui_MixNode },
	{ "SGR", "Simple Gradient", NodeCtor(SimpleGradientNode, generatorNodeColor), gui_SimpleGradientNode },
	{ "", "", nullptr, nullptr }
};

static Control* createTextureNodeEditorGui(GUISystem* gui, VisualNode* ref) {
	for (NodeContructor ctor : nodeTypes) {
		if (ctor.code == ref->code()) {
			return ctor.onGui != nullptr ? ctor.onGui(gui, ref) : nullptr;
		}
	}
	return nullptr;
}

static VisualTextureNode* createNewTextureNode(NodeEditor* editor, const std::string& code) {
	for (NodeContructor ctor : nodeTypes) {
		if (ctor.code == code) {
			return ctor.onCreate(editor, ctor.name, ctor.code);
		}
	}
	return nullptr;
}
