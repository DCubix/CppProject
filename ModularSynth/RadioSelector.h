#pragma once

#include "Control.h"
#include "Animator.h"

#include <map>
#include <string>

constexpr float dur = 0.08f;

class RadioSelector : public Control {
public:
	void onDraw(NVGcontext* ctx, float deltaTime);

	void onMouseDown(int button, int x, int y) override;
	void onMouseMove(int x, int y, int dx, int dy) override;
	void onMouseLeave() override;
	
	int selected() const { return m_selectedValue; }
	void select(int value) {
		m_clickAnimators[m_selectedValue].target(0.0f, dur);
		m_selectedValue = value;
		m_clickAnimators[m_selectedValue].target(1.0f, dur);
		m_hoverAnimators[m_selectedValue].target(0.0f, 0.01f);
	}

	void addOption(int value, const std::string& description);

	std::function<void(int)> onSelect{ nullptr };
	
	const std::map<int, std::string>& options() const { return m_options; }
private:
	std::map<int, std::string> m_options;
	std::map<int, Animator<float>> m_clickAnimators;
	std::map<int, Animator<float>> m_hoverAnimators;
	std::map<int, bool> m_buttonStates;

	int m_hoveredValue{ -1 }, m_selectedValue{ -1 };

	int getOption(int x, int y);
};
