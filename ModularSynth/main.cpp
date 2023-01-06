#define SHOW_FPS 1
#include "Application.h"
#include "GUISystem.h"

#include "Label.h"
#include "Button.h"
#include "Slider.h"
#include "Knob.h"

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

		ControlTest* ctrl = new ControlTest();
		gui->addControl(ctrl);

		Label* lbl = new Label();
		gui->addControl(lbl);

		Button* btn = new Button();
		gui->addControl(btn);

		Slider* sld = new Slider();
		gui->addControl(sld);

		Knob* knb = new Knob();
		gui->addControl(knb);

		lbl->text = "Hello World! Testing stuff...";
		lbl->bounds = { 50, 150, 200, 40 };

		btn->text = "Basic Button";
		btn->bounds = { 250, 150, 140, 40 };

		sld->bounds = { 250, 200, 140, 40 };
		sld->step = 0.01f;
		sld->valueFormat = "{:.2f} sec.";

		knb->bounds = { 250, 250, 72, 72 };
		knb->step = 0.01f;
		knb->valueFormat = "{:.1f}%";
		knb->max = 100.0f;
		knb->step = 0.5f;

		ctrl->bounds = { 50, 50, 120, 90 };
	}

	void onUpdate(Application& app, float dt) {
		auto [width, height] = app.window().size();

		glClearColor(0.1f, 0.2f, 0.4f, 1.0f);
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

};

int main(int argc, char** argv) {
	Application* app = new Application(new Test());
	return app->run();
}
