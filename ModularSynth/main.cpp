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
#include "TextureNodeGraph.hpp"

#include "ShaderGen.h"

#include "nanovg/nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg/nanovg_gl.h"

#include <format>
#include <sstream>
#include <array>

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

		Panel* pnlSettings = new Panel();
		pnlSettings->title = "Settings";
		pnlSettings->setLayout(new ColumnLayout());
		gui->addControl(pnlSettings);

		Panel* pnlControls = new Panel();
		pnlControls->title = "Controls";
		pnlControls->setLayout(new ColumnLayout());
		gui->addControl(pnlControls);

		NodeEditor* ned = new NodeEditor(new TextureNodeGraph());
		graph = static_cast<TextureNodeGraph*>(ned->graph());

		ned->bounds = { 340, 12, int(app.window().size().first) - 352, int(app.window().size().second) - 24 };
		gui->addControl(ned);

		ned->onSelect = [=](VisualNode* node) {
			if (singleNodeEditor) {
				pnlSettings->removeChild(singleNodeEditor);
				gui->removeControl(singleNodeEditor->id());
				singleNodeEditor = nullptr;
			}
			singleNodeEditor = createTextureNodeEditorGui(gui, node);
			if (singleNodeEditor) {
				pnlSettings->addChild(singleNodeEditor);
			}
		};

		ned->onParamChange = [=]() {
			graph->render();
		};

		// build the Node list UI
		for (NodeContructor ctor : nodeTypes) {
			if (!ctor.onCreate) break;

			Button* btn = new Button();
			gui->addControl(btn);
			btn->text = std::format("[+] {}", ctor.name);
			btn->onPress = [=]() {
				createNewTextureNode(ned, ctor.code);
			};
			btn->bounds = { 0, 0, 0, 24 };
			pnlControls->addChild(btn);
		}

		int hh = app.window().size().second / 2;
		pnlSettings->bounds = { 12, 12, 320, hh - 12 };
		pnlControls->bounds = { 12, hh + 12, 320, hh - 36 };

		int wgCount[3], wgSize[3];
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &wgCount[0]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &wgCount[1]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &wgCount[2]);

		std::cout << "Work group count: " << wgCount[0] << ", " << wgCount[1] << ", " << wgCount[2] << '\n';

		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &wgSize[0]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &wgSize[1]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &wgSize[2]);

		std::cout << "Work group size: " << wgSize[0] << ", " << wgSize[1] << ", " << wgSize[2] << '\n';

	}

	void onUpdate(Application& app, float dt) {
		auto [width, height] = app.window().size();

		glClearColor(bgColor[0], bgColor[1], bgColor[2], 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		nvgBeginFrame(ctx, width, height, 1.0f);

		gui->renderAll(ctx, dt);

		if (graph->output) {
			if (image < 0) {
				image = nvglCreateImageFromHandleGL3(
					ctx,
					graph->output->id(),
					graph->output->size()[0],
					graph->output->size()[1], 0
				);
			}

			int x = app.window().size().first - 200;
			int y = app.window().size().second - 200;
			NVGpaint imgPaint = nvgImagePattern(ctx, x, y, 200, 200, 0.0f, image, 1.0f);
			nvgBeginPath(ctx);
			nvgRect(ctx, x, y, 200, 200);
			nvgFillPaint(ctx, imgPaint);
			nvgFill(ctx);
		}

		nvgEndFrame(ctx);
	}

	void onExit() {
		nvgDeleteGL3(ctx);
		delete gui;
	}

	NVGcontext* ctx;

	TextureNodeGraph* graph;

	GUISystem* gui;
	Control* singleNodeEditor{ nullptr };
	float bgColor[3] = { 0.1f, 0.2f, 0.4f };

	int image{ -1 };
};

int main(int argc, char** argv) {
#if 1
	Application* app = new Application(new Test());
	return app->run();
#else

	ShaderGen gen{};

	std::string libTest = R"(void mix_blend(float fac, vec4 col1, vec4 col2, out vec4 outcol)
{
	fac = clamp(fac, 0.0, 1.0);
	outcol = mix(col1, col2, fac);
	outcol.a = col1.a;
}

void mix_add(float fac, vec4 col1, vec4 col2, out vec4 outcol)
{
	fac = clamp(fac, 0.0, 1.0);
	outcol = mix(col1, col1 + col2, fac);
	outcol.a = col1.a;
}

void mix_mult(float fac, vec4 col1, vec4 col2, out vec4 outcol)
{
	fac = clamp(fac, 0.0, 1.0);
	outcol = mix(col1, col1 * col2, fac);
	outcol.a = col1.a;
}

void mix_screen(float fac, vec4 col1, vec4 col2, out vec4 outcol)
{
	fac = clamp(fac, 0.0, 1.0);
	float facm = 1.0 - fac;

	outcol = vec4(1.0) - (vec4(facm) + fac * (vec4(1.0) - col2)) * (vec4(1.0) - col1);
	outcol.a = col1.a;
}

void mix_hue(float fac, vec4 col1, vec4 col2, out vec4 outcol)
{
	fac = clamp(fac, 0.0, 1.0);
	float facm = 1.0 - fac;

	outcol = col1;

	vec4 hsv, hsv2, tmp;
	rgb_to_hsv(col2, hsv2);

	if (hsv2.y != 0.0) {
		rgb_to_hsv(outcol, hsv);
		hsv.x = hsv2.x;
		hsv_to_rgb(hsv, tmp);

		outcol = mix(outcol, tmp, fac);
		outcol.a = col1.a;
	}
}
)";
	gen.loadLib(libTest);
#endif
	return 0;
}
