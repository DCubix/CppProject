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
#include "Edit.h"
#include "TextureView.h"

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
#include <string_view>

#include <iostream>

struct MenuItem {
	std::string text;
	std::function<void()> action;
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

		// UI layout
		SlicedRect layoutMain{ 0, 0, app.window().size().first, app.window().size().second };

		SlicedRect topBar = layoutMain.cutTop(42);
		SlicedRect bodyArea = layoutMain;

		SlicedRect sideBarArea = bodyArea.cutLeft(320);
		SlicedRect nodeGraphArea = bodyArea;

		SlicedRect controlsArea = sideBarArea.cutTop(320);
		SlicedRect settingsArea = sideBarArea;
		//

		gui = new GUISystem();
		gui->attachToApplication(app);

		Panel* pnlSettings = new Panel();
		pnlSettings->title = "Settings";
		pnlSettings->setLayout(new ColumnLayout());
		pnlSettings->bounds = settingsArea.toRect().inflate(-4);
		gui->addControl(pnlSettings);

		Panel* pnlControls = new Panel();
		pnlControls->title = "Controls";
		pnlControls->setLayout(new ColumnFlowLayout());
		pnlControls->bounds = controlsArea.toRect().inflate(-4);
		gui->addControl(pnlControls);

		Panel* pnlMenu = new Panel();
		pnlMenu->drawBackground(false);
		pnlMenu->bounds = topBar.toRect().inflate(-4);
		gui->addControl(pnlMenu);

		// menus
		MenuItem menu[] = {
			{ "Open", [=]() { menu_OpenGraph(); } },
			{ "Save", [=]() { menu_SaveGraph(); } },
		};

		for (const auto& item : menu) {
			Button* menuButton = new Button();
			menuButton->text = item.text;
			menuButton->bounds = topBar.cutLeft(80).toRect().inflate(-4);
			menuButton->onPress = item.action;
			gui->addControl(menuButton);
			pnlMenu->addChild(menuButton);
		}
		//

		ned = new NodeEditor(new TextureNodeGraph());
		graph = static_cast<TextureNodeGraph*>(ned->graph());

		ned->bounds = nodeGraphArea.toRect().inflate(-4);
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

			OutputNode* out = dynamic_cast<OutputNode*>(node->node());
			if (out) {
				if (!out->texture) return;

				previewControl->setTexture(out->texture.get());
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
			btn->text = ctor.name;
			btn->onPress = [=]() {
				auto node = createNewTextureNode(ned, ctor.code);
				nodeTypeStorage[node->node()->id()] = { ctor.code, node->id() };
			};
			btn->bounds = { 0, 0, 0, 24 };
			pnlControls->addChild(btn);
		}

		// preview
		Panel* pnlPreview = new Panel();
		pnlPreview->title = "Preview";
		pnlPreview->bounds = { int(app.window().size().first) - 268, int(app.window().size().second) - 268, 256, 256 };
		pnlPreview->setLayout(new RowLayout(1));
		gui->addControl(pnlPreview);

		previewControl = new TextureView();
		previewControl->bounds = { 8, 8, pnlPreview->bounds.width - 16, pnlPreview->bounds.height - 16 };
		previewControl->setOrder(999);
		gui->addControl(previewControl);
		pnlPreview->addChild(previewControl);
		//

		/*int wgCount[3], wgSize[3];
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &wgCount[0]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &wgCount[1]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &wgCount[2]);

		std::cout << "Work group count: " << wgCount[0] << ", " << wgCount[1] << ", " << wgCount[2] << '\n';

		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &wgSize[0]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &wgSize[1]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &wgSize[2]);

		std::cout << "Work group size: " << wgSize[0] << ", " << wgSize[1] << ", " << wgSize[2] << '\n';*/

		if(const auto& args = app.args(); args.size() > 1) {

			if(!openNodeGraph(args[1])) {
				auto msg = std::format("Failed to open file '{}' !", args[1]);
				pfd::message message("Error!", msg, pfd::choice::ok, pfd::icon::error);
			}

		}

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

	void menu_OpenGraph() {
		auto fp = pfd::open_file(
			"Open Node Graph",
			pfd::path::home(),
			{ "Node Graph Files", "*.dat" },
			pfd::opt::none
		);
		if (!fp.result().empty()) {
			openNodeGraph(fp.result().front());
		}
	}

	void menu_SaveGraph() {
		olc::utils::datafile out{};

		graph->save(out);
		for (auto& [nodeId, p] : nodeTypeStorage) {
			auto [type, visualNodeId] = p;
			out["nodes"][std::format("node_{}", nodeId)]["type"].SetString(type);

			auto vnode = ned->get(visualNodeId);
			out["nodes"][std::format("node_{}", nodeId)]["position"].SetInt(vnode->position.x, 0);
			out["nodes"][std::format("node_{}", nodeId)]["position"].SetInt(vnode->position.y, 1);
		}

		auto fp = pfd::save_file(
			"Save Node Graph",
			pfd::path::home(),
			{ "Node Graph Files", "*.dat" },
			pfd::opt::none
		);
		if (!fp.result().empty()) {
			out.Write(out, fp.result());
		}
	}

	bool openNodeGraph(const std::string_view& file) {

		olc::utils::datafile in{};
		if(!in.Read(in, std::string(file)))
			return false;

		// create nodes
		for (size_t i = 0; i < in["nodes"].GetArraySize(); i++) {
			auto&& val = in["nodes"].GetArrayItem(i);
			auto&& node = createNewTextureNode(ned, val["type"].GetString());
			node->position.x = val["position"].GetInt(0);
			node->position.y = val["position"].GetInt(1);
			static_cast<GraphicsNode*>(node->node())->loadFrom(val);

			nodeTypeStorage[node->node()->id()] = { val["type"].GetString(), node->id() };
		}

		for (size_t i = 0; i < in["connections"].GetArraySize(); i++) {
			auto&& val = in["connections"].GetArrayItem(i);
			ned->connect(
				ned->getFromOriginalNodeId(val["source"].GetInt()),
				val["sourceOutput"].GetInt(),
				ned->getFromOriginalNodeId(val["destination"].GetInt()),
				val["destinationInput"].GetInt()
			);
		}

		return true;

	}

	NVGcontext* ctx;

	NodeEditor* ned;
	TextureNodeGraph* graph;
	std::map<size_t, std::pair<std::string, size_t>> nodeTypeStorage;

	GUISystem* gui;
	Control* singleNodeEditor{ nullptr };

	float bgColor[3] = { 0.1f, 0.2f, 0.4f };

	TextureView* previewControl;
};

int main(int argc, char** argv) {
#if 1
	Application* app = new Application(new Test(), argc, argv);
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
