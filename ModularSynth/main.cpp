#define SHOW_FPS 1
#include "Application.h"
#include "GUISystem.h"

#include "Label.h"
#include "Button.h"
#include "Slider.h"
#include "Knob.h"
#include "CheckBox.h"
#include "RadioSelector.h"
#include "Panel.h"
#include "NodeEditor.h"

#include "Animator.h"

#include "GraphicsNode.h"
#include "TextureNodeRegistry.h"

#include "nanovg/nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg/nanovg_gl.h"

#include <format>
#include <sstream>

#include <iostream>

class ControlTest : public Control {
public:

	void onDraw(NVGcontext* ctx, float deltaTime) override {
		float v = anim.value(Curves::easeInOutCubic, deltaTime);
		Rect b = bounds;
		nvgBeginPath(ctx);
		nvgRoundedRect(ctx, v, v, b.width - v*2, b.height - v*2, 6.0f);
		nvgStrokeColor(ctx, nvgRGB(0, 255, 255));
		nvgStroke(ctx);
	}

	void onMouseDown(int button, int x, int y) override {
		anim.forward(3.0f, 0.0f, 0.3f);
	}

	void onMouseUp(int button, int x, int y) override {
		anim.reverse(0.3f);
	}

	Animator<float> anim{};

};


class Test : public ApplicationAdapter {
public:
	WindowParams onSetup() {
		return {
			.title = TEXT("Application Test!")
		};
	}

	void onStart(Application& app) {
		ctx = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);

		nvgCreateFont(ctx, "default-bold", "fonts/os_bold.ttf");
		nvgCreateFont(ctx, "default-bold-italic", "fonts/os_bold_italic.ttf");
		nvgCreateFont(ctx, "default-italic", "fonts/os_italic.ttf");
		nvgCreateFont(ctx, "default", "fonts/os_regular.ttf");
		nvgFontFace(ctx, "default");

		gui = new GUISystem();
		gui->attachToApplication(app);

		Panel* pnl = new Panel();
		pnl->title = "Settings";
		pnl->setLayout(new ColumnLayout());
		gui->addControl(pnl);

		NodeEditor* ned = new NodeEditor();
		ned->bounds = { 270, 12, int(app.window().size().first) - 282, int(app.window().size().second) - 24 };
		gui->addControl(ned);

		ned->onSelect = [=](VisualNode* node) {
			if (singleNodeEditor) {
				pnl->removeChild(singleNodeEditor);
				gui->removeControl(singleNodeEditor->id());
				singleNodeEditor = nullptr;
			}
			singleNodeEditor = createTextureNodeEditorGui(gui, node);
			if (singleNodeEditor) {
				pnl->addChild(singleNodeEditor);
			}
		};

		/* NODE TEST */
		auto col1 = createNewTextureNode(ned, "COL");
		((GraphicsNode*)col1->node())->setParam("Color", { 1.0f, 0.0f, 0.0f, 1.0f });
		auto col2 = createNewTextureNode(ned, "COL");
		((GraphicsNode*)col2->node())->setParam("Color", { 0.0f, 1.0f, 0.0f, 1.0f });

		auto sgr = createNewTextureNode(ned, "SGR");
		auto mix = createNewTextureNode(ned, "MIX");

		//ned->connect(col1, 0, mix, 0);
		//ned->connect(col2, 0, mix, 1);
		//ned->connect(sgr, 0, mix, 2);

		/* --------- */

		pnl->bounds = { 12, 12, 250, int(app.window().size().second) - 24 };
	}

	void onUpdate(Application& app, float dt) {
		auto [width, height] = app.window().size();

		glClearColor(bgColor[0], bgColor[1], bgColor[2], 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		nvgBeginFrame(ctx, width, height, 1.0f);

		gui->renderAll(ctx, dt);

		nvgEndFrame(ctx);
	}

	void onExit() {
		nvgDeleteGL3(ctx);
		delete gui;
	}

	NVGcontext* ctx;

	GUISystem* gui;
	Control* singleNodeEditor{ nullptr };
	float bgColor[3] = { 0.1f, 0.2f, 0.4f };

	int image;
};

int main(int argc, char** argv) {
	Application* app = new Application(new Test());
	return app->run();
}
