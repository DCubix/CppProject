#pragma once

#include "Control.h"
#include <memory>

class Application;

class GUISystem {
public:

	void addControl(Control* control);
	void renderAll(NVGcontext* ctx, float deltaTime);

	MouseButtonEventSystem& mouseEvents() { return *m_mouseButtonEventSystem; }
	MouseMotionEventSystem& motionEvents() { return *m_mouseMotionEventSystem; }
	KeyboardEventSystem& keyboardEvents() { return *m_keyboardEventSystem; }

	void attachToApplication(Application& app);

private:
	std::vector<std::unique_ptr<Control>> m_controls;

	MouseButtonEventSystem* m_mouseButtonEventSystem{};
	MouseMotionEventSystem* m_mouseMotionEventSystem{};
	KeyboardEventSystem* m_keyboardEventSystem{};
};

