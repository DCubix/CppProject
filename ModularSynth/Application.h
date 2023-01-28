#pragma once

#include <memory>
#include <vector>
#include <string_view>
#include <span>

#include "Window.h"

class Application;
class ApplicationAdapter {
public:
	virtual WindowParams onSetup() = 0;
	virtual void onStart(Application& app) = 0;
	virtual void onUpdate(Application& app, float deltaTime) = 0;
	virtual void onEvent(WindowEvent ev) {}
	virtual void onExit() = 0;
};

class Application {
public:
	Application() = default;
	~Application() = default;

	Application(ApplicationAdapter* adapter, int argc = 0, char** argv = nullptr);

	int run(int fpsCap = -1);

	Window& window() { return *m_window.get(); }

	inline const std::vector<std::string_view>& args() const { return m_arguments; }

private:
	std::vector<std::string_view> m_arguments;
	std::unique_ptr<Window> m_window;
	std::unique_ptr<ApplicationAdapter> m_adapter;
	float m_frameTime{ 0.0f };
	int m_frameCount{ 0 };
};
