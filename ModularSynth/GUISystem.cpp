#include "GUISystem.h"

#include "Application.h"

void GUISystem::addControl(Control* control) {
	m_controls.push_back(std::unique_ptr<Control>(control));

	auto ctrl = m_controls.back().get();
	m_mouseButtonEventSystem->addListener(ctrl);
	m_mouseMotionEventSystem->addListener(ctrl);
	m_keyboardEventSystem->addListener(ctrl);
}

void GUISystem::renderAll(NVGcontext* ctx, float deltaTime) {
	for (auto&& ctrl : m_controls) {
		nvgSave(ctx);

		// make local coordinates
		Rect bounds = ctrl->screenSpaceBounds();
		nvgTranslate(ctx, bounds.x, bounds.y);

		ctrl->onDraw(ctx, deltaTime);
		nvgRestore(ctx);
	}

	for (auto&& ctrl : m_controls) {
		nvgSave(ctx);

		// make local coordinates
		Rect bounds = ctrl->screenSpaceBounds();
		nvgTranslate(ctx, bounds.x, bounds.y);

		ctrl->onPostDraw(ctx, deltaTime);
		nvgRestore(ctx);
	}
}

void GUISystem::attachToApplication(Application& app) {
	m_mouseButtonEventSystem = &app.mouseEvents();
	m_mouseMotionEventSystem = &app.motionEvents();
	m_keyboardEventSystem = &app.keyboardEvents();
}
