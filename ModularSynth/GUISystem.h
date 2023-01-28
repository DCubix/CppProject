#pragma once

#include "Control.h"

#include <map>
#include <memory>

#include "nanovg/nanovg.h"

template <typename T>
concept ControlType = std::is_base_of<Control, T>::value;

class GUISystem {
public:
	GUISystem();
	~GUISystem();

	void onEvent(WindowEvent ev);
	void onDraw(int width, int height, float deltaTime);

	std::shared_ptr<Control> root() { return m_root; }

	template <ControlType Ctrl, typename... Args>
	Ctrl* create(Args&&... args) {
		Ctrl* ctrl = new Ctrl(std::forward<Args>(args)...);
		root()->addChild(ctrl);
		return ctrl;
	}

private:
	std::shared_ptr<Control> m_root;

	NVGcontext* m_context{ nullptr };
};

