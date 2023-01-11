#pragma once

#include "TextureNodes.hpp"
#include "NodeEditor.h"

#include "GUISystem.h"
#include "Slider.h"
#include "Panel.h"

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
	pnl->bounds = { 0, 0, 0, 154 };
	pnl->setLayout(new ColumnLayout());
	gui->addControl(pnl);

	GraphicsNode* nd = (GraphicsNode*)node->node();
	for (size_t i = 0; i < 4; i++) {
		Slider* sld = new Slider();
		gui->addControl(sld);

		sld->value = nd->param("Color").value[i];
		sld->onChange = [=](float v) {
			nd->setParam("Color", i, v);
		};

		sld->bounds = { 0, 0, 0, 38 };
		pnl->addChild(sld);
	}

	return pnl;
}

static Control* gui_SimpleGradientNode(GUISystem* gui, VisualNode* node) {
	Panel* pnl = new Panel();
	pnl->drawBackground(false);
	pnl->bounds = { 0, 0, 0, 40 };
	pnl->setLayout(new ColumnLayout());
	gui->addControl(pnl);

	GraphicsNode* nd = (GraphicsNode*)node->node();
	Slider* sld = new Slider();
	gui->addControl(sld);

	sld->value = nd->param("Angle").value[0];
	sld->onChange = [=](float v) {
		nd->setParam("Angle", v);
	};
	sld->min = -PI;
	sld->max = PI;
	sld->bounds = { 0, 0, 0, 38 };

	pnl->addChild(sld);

	return pnl;
}

static NodeContructor nodeTypes[] = {
	{ "COL", "Color", NodeCtor(ColorNode, generatorNodeColor), gui_ColorNode },
	{ "MIX", "Mix", NodeCtor(MixNode, operatorNodeColor) },
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
