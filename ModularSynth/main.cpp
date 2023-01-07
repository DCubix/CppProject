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

#include "Animator.h"

#include "nanovg/nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg/nanovg_gl.h"

#include <format>
#include <sstream>

#include <iostream>

class ControlTest : public Control {
public:

	void onDraw(NVGcontext* ctx, float deltaTime) {
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

		Label* lbl = new Label();
		gui->addControl(lbl);

		Button* btn = new Button();
		gui->addControl(btn);

		Slider* sld = new Slider();
		gui->addControl(sld);

		CheckBox* ckb = new CheckBox();
		gui->addControl(ckb);

		RadioSelector* rse = new RadioSelector();
		gui->addControl(rse);

		Panel* pnl = new Panel();
		gui->addControl(pnl);

		pnl->setLayout(new ColumnLayout());

		pnl->addChild(lbl);
		pnl->addChild(btn);
		pnl->addChild(sld);
		pnl->addChild(rse);
		pnl->addChild(ckb);

		Panel* pnl2 = new Panel();
		gui->addControl(pnl2);
		pnl2->setLayout(new RowLayout(3));
		pnl2->drawBackground(false);
		pnl2->bounds = { 0, 0, 0, 64 };
		for (int i = 0; i < 3; i++) {
			Knob* knb = new Knob();
			gui->addControl(knb);
			pnl2->addChild(knb);

			knb->bounds = { 0, 0, 48, 48 };
			knb->step = 0.01f;
			knb->value = bgColor[i];
			knb->onChange = [=](float v) {
				bgColor[i] = v;
			};
		}

		pnl->addChild(pnl2);

		pnl->bounds = { 12, 12, 250, int(app.window().size().second - 24) };

		lbl->text = "Hello World! Testing stuff...";
		lbl->bounds = { 0, 0, 200, 40 };
		lbl->alignment = HorizontalAlignment::center;

		btn->text = "Basic Button";
		btn->bounds = { 0, 0, 140, 40 };

		sld->bounds = { 0, 50, 140, 40 };
		sld->step = 0.01f;
		sld->valueFormat = "{:.2f} sec.";

		ckb->text = "Check It Out";
		ckb->bounds = { 0, 0, 120, 26 };

		rse->addOption(1, "Option 1");
		rse->addOption(2, "Option 2");
		rse->addOption(3, "Option 3");
		rse->bounds = { 0, 0, 300, 25 };
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
	float bgColor[3] = { 0.1f, 0.2f, 0.4f };
};

int main(int argc, char** argv) {
	Application* app = new Application(new Test());
	return app->run();
}
