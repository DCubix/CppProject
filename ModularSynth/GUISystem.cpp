#include "GUISystem.h"

#include "Application.h"
#include "Panel.h"

#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg/nanovg_gl.h"

GUISystem::GUISystem() {
	Panel* _root = new Panel();
	_root->drawBackground(false);
	m_root = std::shared_ptr<Control>(_root);
	m_root->m_id = 1;

	m_context = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);

	nvgCreateFont(m_context, "default-bold", "fonts/os_bold.ttf");
	nvgCreateFont(m_context, "default-bold-italic", "fonts/os_bold_italic.ttf");
	nvgCreateFont(m_context, "default-italic", "fonts/os_italic.ttf");
	nvgCreateFont(m_context, "default", "fonts/os_regular.ttf");
	nvgFontFace(m_context, "default");

}

GUISystem::~GUISystem() {
	nvgDeleteGL3(m_context);
}

void GUISystem::onEvent(WindowEvent ev) {
	m_root->onEvent(ev);
}

void GUISystem::onDraw(int width, int height, float deltaTime) {
	m_root->flush();

	nvgBeginFrame(m_context, width, height, 1.0f);
	m_root->bounds = { 0, 0, width, height };
	m_root->onDraw(m_context, deltaTime);
	m_root->onPostDraw(m_context, deltaTime);
	m_root->bounds = { 0, 0, width, height };
	nvgEndFrame(m_context);
}
