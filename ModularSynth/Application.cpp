#include "Application.h"

#include <chrono>
using namespace std::chrono;

#include <iostream>

#ifdef _DEBUG
#include <format>
#endif

static double currentTimeMillis() {
	auto now = steady_clock::now();
	return duration_cast<milliseconds>(now.time_since_epoch()).count() / 1000.0;
}

Application::Application(ApplicationAdapter* adapter, int argc, char** argv) {
	for(int i = 0; i < argc; i++)
		m_arguments.push_back(argv[i]);
	m_adapter = std::unique_ptr<ApplicationAdapter>(adapter);
}

int Application::run(int fpsCap) {
	if (!m_adapter) return 1;

	WindowParams params = m_adapter->onSetup();
	m_window = std::make_unique<Window>();
	if (!m_window->create(params)) {
		return 1;
	}

	const double timeStep = 1.0 / fpsCap;
	double lastTime = currentTimeMillis();

	WindowEvent ev;

	m_adapter->onStart(*this);
	while (m_window->pollEvents(ev)) {
		double currentTime = currentTimeMillis();
		double deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		// Event processing
		m_adapter->onEvent(ev);

		m_adapter->onUpdate(*this, float(deltaTime));
		m_window->swapBuffers();

		m_frameTime += float(deltaTime);
		m_frameCount++;
		if (m_frameTime >= 1.0f) {
#ifdef _DEBUG
			m_window->title(std::format(TEXT("!!DEBUG MODE!! - {} - [{} FPS]"), params.title, m_frameCount));
#else
			m_window->title(std::format(TEXT("{} - [{} FPS]"), params.title, m_frameCount));
#endif

			m_frameCount = 0;
			m_frameTime -= 1.0f;
		}
	}
	m_adapter->onExit();

	return 0;
}
