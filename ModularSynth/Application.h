#pragma once

#include <memory>

#include "Window.h"
#include "EventSystem.h"

class Application;
class ApplicationAdapter {
public:
	virtual WindowParams onSetup() = 0;
	virtual void onStart(Application& app) = 0;
	virtual void onUpdate(Application& app, float deltaTime) = 0;
	virtual void onExit() = 0;
};

class Application {
public:
	Application() = default;
	~Application() = default;

	Application(ApplicationAdapter* adapter);

	int run(int fpsCap = -1);

	Window& window() { return *m_window.get(); }

	MouseButtonEventSystem& mouseEvents() { return m_mouseButtonEventSystem; }
	MouseMotionEventSystem& motionEvents() { return m_mouseMotionEventSystem; }
	KeyboardEventSystem& keyboardEvents() { return m_keyboardEventSystem; }

private:
	std::unique_ptr<Window> m_window;
	std::unique_ptr<ApplicationAdapter> m_adapter;
	float m_frameTime{ 0.0f };
	int m_frameCount{ 0 };

	MouseButtonEventSystem m_mouseButtonEventSystem{};
	MouseMotionEventSystem m_mouseMotionEventSystem{};
	KeyboardEventSystem m_keyboardEventSystem{};
};
