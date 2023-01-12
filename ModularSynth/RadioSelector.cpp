#include "RadioSelector.h"

static void drawButton(
	NVGcontext* ctx,
	const std::string& text,
	float x, float y, float w, float h,
	float radiusLeft = 12.0f, float radiusRight = 12.0f,
	float bgBright = 0.0f,
	float borderAlpha = 0.0f)
{
	float bgAlpha = LERP(0.6f, 1.0f, bgBright);
	nvgBeginPath(ctx);
	nvgRoundedRectVarying(ctx, x, y, w, h, radiusLeft, radiusRight, radiusRight, radiusLeft);
	nvgFillColor(ctx, nvgRGBAf(bgBright, bgBright, bgBright, bgAlpha));
	if (borderAlpha >= 1e-5f) {
		nvgStrokeWidth(ctx, 1.0f);
		nvgStrokeColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, borderAlpha));
		nvgStroke(ctx);
	}
	nvgFill(ctx);

	nvgFillColor(ctx, nvgRGBf(1.0f - bgBright, 1.0f - bgBright, 1.0f - bgBright));
	nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
	nvgFontSize(ctx, 14.0f);
	nvgTextBox(ctx, x, y + h / 2 + 1.5f, w, text.c_str(), nullptr);
}

void RadioSelector::onDraw(NVGcontext* ctx, float deltaTime) {
	Rect b = bounds;

	if (m_options.size() == 1) {
		float hoverValue = m_hoverAnimators[m_options.begin()->first].value(Curves::easeInOutQuad, deltaTime);
		float clickValue = m_clickAnimators[m_options.begin()->first].value(Curves::easeInOutQuad, deltaTime);
		drawButton(ctx, m_options.begin()->second, 0.0f, 0.0f, b.width, b.height, 12.0f, 12.0f, clickValue, hoverValue);
	}
	else if (m_options.size() > 1) {
		int i = 0;
		const float buttonWidth = b.width / m_options.size();
		for (auto&& [value, description] : m_options) {
			float radiusLeft = i == 0 ? 12.0f : 0.0f;
			float radiusRight = i >= m_options.size() - 1 ? 12.0f : 0.0f;

			float hoverValue = m_hoverAnimators[value].value(Curves::easeInOutQuad, deltaTime);
			float clickValue = m_clickAnimators[value].value(Curves::easeInOutQuad, deltaTime);

			drawButton(
				ctx,
				description,
				buttonWidth * i, 0.0f,
				buttonWidth, b.height,
				radiusLeft, radiusRight,
				clickValue, hoverValue
			);

			if (i > 0 && i < m_options.size()) {
				nvgBeginPath(ctx);
				nvgMoveTo(ctx, buttonWidth * i, 1.0f);
				nvgLineTo(ctx, buttonWidth * i, b.height - 1.0f);
				nvgStrokeWidth(ctx, 0.7f);
				nvgStrokeColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 0.7f));
				nvgStroke(ctx);
			}

			i++;
		}
	}
}

void RadioSelector::onMouseDown(int btn, int x, int y) {
	const int button = getOption(x, y);
	if (m_selectedValue != button) {
		m_clickAnimators[m_selectedValue].reverse(dur);

		m_selectedValue = button;
		m_clickAnimators[m_selectedValue].forward(1.0f, 0.0f, dur);
		m_hoverAnimators[m_selectedValue].reset();

		if (onSelect) onSelect(m_selectedValue);
	}
}

void RadioSelector::onMouseMove(int x, int y, int dx, int dy) {
	const int button = getOption(x, y);

	if (m_hoveredValue != button && m_buttonStates[m_hoveredValue]) {
		m_hoverAnimators[m_hoveredValue].reverse(dur);
		m_buttonStates[m_hoveredValue] = false;
	}

	m_hoveredValue = button;
	if (!m_buttonStates[m_hoveredValue]) {
		m_hoverAnimators[m_hoveredValue].forward(1.0f, 0.0f, dur);
		m_buttonStates[m_hoveredValue] = true;
	}
}

void RadioSelector::onMouseLeave() {
	if (m_hoveredValue != -1) {
		m_hoverAnimators[m_hoveredValue].reverse(dur);
		m_buttonStates[m_hoveredValue] = false;
	}
}

void RadioSelector::addOption(int value, const std::string& description) {
	m_options[value] = description;
	m_clickAnimators[value] = Animator<float>();
	m_hoverAnimators[value] = Animator<float>();
	m_buttonStates[value] = false;
}

int RadioSelector::getOption(int x, int y) {
	Rect b = bounds;
	if (m_options.empty()) return -1;

	int i = 0;
	const float buttonWidth = b.width / m_options.size();
	for (auto&& [value, description] : m_options) {
		Rect buttonRect = { i * buttonWidth, 0, buttonWidth, b.height };
		if (buttonRect.hasPoint({ x, y })) {
			return value;
		}
		i++;
	}

	return -1;
}
