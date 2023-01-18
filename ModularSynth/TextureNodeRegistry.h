#pragma once

#include "TextureNodes.hpp"
#include "NodeEditor.h"

#include "GUISystem.h"
#include "Slider.h"
#include "Panel.h"
#include "Label.h"
#include "RadioSelector.h"
#include "Button.h"

#include "portable-file-dialogs.h"

#include "nanovg/stb_image.h"

#define hex2rgbf(h) { float((h & 0xFF0000) >> 16) / 255.0f, float((h & 0xFF00) >> 8) / 255.0f, float(h & 0xFF) / 255.0f, 1.0f }

static constexpr Color generatorNodeColor = hex2rgbf(0x47b394);
static constexpr Color operatorNodeColor = hex2rgbf(0xb86335);
static constexpr Color externalNodeColor = hex2rgbf(0x7a30ba);

/* UI Editing */
using GuiBuilder = std::function<Control* (GUISystem*, VisualNode*)>;

/* ========== */

using NodeBuilder = std::function<VisualNode* (NodeEditor*, const std::string&, const std::string&)>;

struct NodeContructor {
	std::string code, name;
	NodeBuilder onCreate;
	GuiBuilder onGui;
};

#define NodeCtor(T, c) [](NodeEditor* editor, const std::string& name, const std::string& code) { return editor->create<T, VisualNode>(name, code, c); }

static Control* gui_ValueSlider(
	GUISystem* gui,
	const std::string& label,
	float value,
	const std::function<void(float)> setter,
	float min = 0.0f,
	float max = 1.0f,
	float step = 0.05f
) {
	Slider* sld = new Slider();
	gui->addControl(sld);

	sld->min = min;
	sld->max = max;
	sld->step = step;

	sld->value = value;
	sld->onChange = [=](float v) {
		setter(v);
	};

	Label* lbl = new Label();
	lbl->text = label;
	lbl->alignment = HorizontalAlignment::right;
	gui->addControl(lbl);

	Panel* row = new Panel();
	row->drawBackground(false);
	row->bounds = { 0, 0, 0, 30 };

	RowLayout* rl = new RowLayout(2, 3);
	rl->expansion[0] = 0.7f;
	rl->expansion[1] = 1.3f;

	row->setLayout(rl);
	row->addChild(lbl);
	row->addChild(sld);

	gui->addControl(row);
	return row;
}

static Control* gui_ColorNode(GUISystem* gui, VisualNode* node) {
	Panel* pnl = new Panel();
	pnl->drawBackground(false);
	pnl->bounds = { 0, 0, 0, 0 };
	pnl->setLayout(new ColumnLayout());
	gui->addControl(pnl);

	const std::string labels[] = { "Red", "Geen", "Blue", "Alpha" };

	GraphicsNode* nd = (GraphicsNode*)node->node();
	for (size_t i = 0; i < 4; i++) {
		auto ctrl = gui_ValueSlider(
			gui, labels[i], nd->param("Color").value[i],
			[=](float v) {
				nd->setParam("Color", i, v);
			}
		);
		pnl->addChild(ctrl);
		pnl->bounds.height += 30;
	}

	return pnl;
}

static Control* gui_MixNode(GUISystem* gui, VisualNode* node) {
	Panel* pnl = new Panel();
	pnl->drawBackground(false);
	pnl->bounds = { 0, 0, 0, 90 };
	pnl->setLayout(new ColumnLayout());
	gui->addControl(pnl);

	GraphicsNode* nd = (GraphicsNode*)node->node();

	RadioSelector* rsel = new RadioSelector();
	rsel->bounds = { 0, 0, 0, 25 };
	rsel->addOption(0, "Mix");
	rsel->addOption(1, "Add");
	rsel->addOption(2, "Sub");
	rsel->addOption(3, "Mul");
	rsel->select(int(nd->param("Mode").value[0]));
	rsel->onSelect = [=](int index) {
		nd->setParam("Mode", float(index));
	};
	gui->addControl(rsel);

	auto ctrl = gui_ValueSlider(
		gui, "Factor", nd->param("Factor").value[0],
		[=](float v) {
			nd->setParam("Factor", v);
		}
	);

	pnl->addChild(rsel);
	pnl->addChild(ctrl);

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

static Control* gui_NoiseNode(GUISystem* gui, VisualNode* node) {
	Panel* pnl = new Panel();
	pnl->drawBackground(false);
	pnl->bounds = { 0, 0, 0, 60 };
	pnl->setLayout(new ColumnLayout());
	gui->addControl(pnl);

	GraphicsNode* nd = (GraphicsNode*)node->node();

	auto scale = gui_ValueSlider(
		gui, "Scale", nd->param("Scale").value[0],
		[=](float v) {
			nd->setParam("Scale", v);
		},
		1.0f, 99.0f
	);
	auto patx = gui_ValueSlider(
		gui, "Pattern X", nd->param("Pattern X").value[0],
		[=](float v) {
			nd->setParam("Pattern X", v);
		}
	);
	auto paty = gui_ValueSlider(
		gui, "Pattern Y", nd->param("Pattern Y").value[0],
		[=](float v) {
			nd->setParam("Pattern Y", v);
		}
	);

	gui->addControl(scale);
	gui->addControl(patx);
	gui->addControl(paty);

	pnl->addChild(scale);
	pnl->addChild(patx);
	pnl->addChild(paty);

	return pnl;
}

static Control* gui_ThresholdNode(GUISystem* gui, VisualNode* node) {
	Panel* pnl = new Panel();
	pnl->drawBackground(false);
	pnl->bounds = { 0, 0, 0, 60 };
	pnl->setLayout(new ColumnLayout());
	gui->addControl(pnl);

	GraphicsNode* nd = (GraphicsNode*)node->node();

	auto thr = gui_ValueSlider(
		gui, "Threshold", nd->param("Threshold").value[0],
		[=](float v) {
			nd->setParam("Threshold", v);
		}
	);
	auto feat = gui_ValueSlider(
		gui, "Feather", nd->param("Feather").value[0],
		[=](float v) {
			nd->setParam("Feather", v);
		}
	);

	gui->addControl(thr);
	gui->addControl(feat);

	pnl->addChild(thr);
	pnl->addChild(feat);

	return pnl;
}

static Control* gui_ImageNode(GUISystem* gui, VisualNode* node) {
	Button* btn = new Button();
	btn->text = "Load Texture";

	ImageNode* nd = (ImageNode*)node->node();
	
	btn->bounds = { 0, 0, 0, 25 };
	btn->onPress = [=]() {
		auto fp = pfd::open_file(
			"Load Image",
			pfd::path::home(),
			{ "Image Files", "*.png *.jpg *.bmp" },
			pfd::opt::none
		);
		if (!fp.result().empty()) {
			if (nd->handle) {
				delete nd->handle;
			}

			int w, h, comp;
			auto data = stbi_loadf(fp.result().front().c_str(), &w, &h, &comp, STBI_rgb_alpha);

			nd->handle = new Texture({ uint32_t(w), uint32_t(h) }, GL_RGBA32F);
			nd->handle->loadFromMemory(data, GL_RGBA, GL_FLOAT);

			nd->setParam("Image", float(nd->handle->id()));
		}
	};
	gui->addControl(btn);
	return btn;
}

static Control* gui_UVNode(GUISystem* gui, VisualNode* node) {
	Panel* pnl = new Panel();
	pnl->drawBackground(false);
	pnl->bounds = { 0, 0, 0, 60 };
	pnl->setLayout(new ColumnLayout());
	gui->addControl(pnl);

	GraphicsNode* nd = (GraphicsNode*)node->node();

	RadioSelector* rsel = new RadioSelector();
	rsel->bounds = { 0, 0, 0, 25 };
	rsel->addOption(0, "Clamp");
	rsel->addOption(1, "Repeat");
	rsel->addOption(2, "Mirror");
	rsel->select(int(nd->param("Repeat").value[0]));
	rsel->onSelect = [=](int index) {
		nd->setParam("Repeat", float(index));
	};
	gui->addControl(rsel);

	auto ctrl = gui_ValueSlider(
		gui, "Strength", nd->param("Strength").value[0],
		[=](float v) {
			nd->setParam("Strength", v);
		}
	);

	pnl->addChild(rsel);
	pnl->addChild(ctrl);

	return pnl;
}

static NodeContructor nodeTypes[] = {
	{ "COL", "Color", NodeCtor(ColorNode, generatorNodeColor), gui_ColorNode },
	{ "MIX", "Mix", NodeCtor(MixNode, operatorNodeColor), gui_MixNode },
	{ "SGR", "Simple Gradient", NodeCtor(SimpleGradientNode, generatorNodeColor), gui_SimpleGradientNode },
	{ "NOI", "Noise", NodeCtor(NoiseNode, generatorNodeColor), gui_NoiseNode },
	{ "THR", "Threshold", NodeCtor(ThresholdNode, operatorNodeColor), gui_ThresholdNode },
	{ "IMG", "Image", NodeCtor(ImageNode, externalNodeColor), gui_ImageNode },
	{ "UVS", "UV", NodeCtor(UVNode, generatorNodeColor), gui_UVNode },
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

static VisualNode* createNewTextureNode(NodeEditor* editor, const std::string& code) {
	for (NodeContructor ctor : nodeTypes) {
		if (ctor.code == code) {
			return ctor.onCreate(editor, ctor.name, ctor.code);
		}
	}
	return nullptr;
}
