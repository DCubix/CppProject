#pragma once

#include "Control.h"
#include <map>
#include <memory>

class Application;
class GUISystem {
public:

	void addControl(Control* control);
	void removeControl(ControlID control);

	void renderAll(NVGcontext* ctx, float deltaTime);

	MouseButtonEventSystem& mouseEvents() { return *m_mouseButtonEventSystem; }
	MouseMotionEventSystem& motionEvents() { return *m_mouseMotionEventSystem; }
	KeyboardEventSystem& keyboardEvents() { return *m_keyboardEventSystem; }

	void attachToApplication(Application& app);

	void begin();
	void end();

private:
	std::map<ControlID, std::shared_ptr<Control>> m_controls;

	std::vector<Control*> m_controlsAdd;
	std::vector<ControlID> m_controlsRemove;

	MouseButtonEventSystem* m_mouseButtonEventSystem{};
	MouseMotionEventSystem* m_mouseMotionEventSystem{};
	KeyboardEventSystem* m_keyboardEventSystem{};

	void createControl(Control* control);
	void deleteControl(ControlID control);

	static ControlID g_ControlID;
};

