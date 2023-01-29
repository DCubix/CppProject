#include "Edit.h"

constexpr float textPad = 8.0f;

void Edit::onDraw(NVGcontext* ctx, float deltaTime) {
	Rect b = bounds;
	nvgBeginPath(ctx);
	nvgRoundedRect(ctx, 0, 0, b.width, b.height, 12.0f);
	nvgFillColor(ctx, nvgRGBAf(0.0f, 0.0f, 0.0f, 0.6f));
	if (m_focused) {
		nvgStrokeColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
		nvgStroke(ctx);
	}
	nvgFill(ctx);

	std::string textP = text + " ";
	m_glyphs.resize(textP.size());

	nvgTextGlyphPositions(ctx, 0.0f, 0.0f, textP.c_str(), nullptr, m_glyphs.data(), textP.size());

	nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
	nvgFontSize(ctx, 16.0f);

	if (!label.empty()) {
		float lblBounds[4] = { 0.0f };
		nvgTextBounds(ctx, 0.0f, 0.0f, label.c_str(), nullptr, lblBounds);
		m_labelOffset = lblBounds[2] - lblBounds[0];

		nvgFillColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 0.5f));
		nvgText(ctx, textPad, b.height / 2 + 1.5f, label.c_str(), nullptr);
	}
	else {
		m_labelOffset = 0.0f;
	}

	float textOffsetX = textPad + m_labelOffset + 4;

	nvgFillColor(ctx, nvgRGBf(1.0f, 1.0f, 1.0f));
	nvgTextBox(ctx, textOffsetX, b.height / 2 + 1.5f, b.width, text.c_str(), nullptr);
	
	// draw cursor
	if (m_cursorShow && m_focused) {
		float cursorX = m_glyphs[m_cursorX].x;

		nvgFillColor(ctx, nvgRGBf(1.0f, 1.0f, 1.0f));
		nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgFontSize(ctx, 16.0f);
		nvgText(ctx, cursorX + textOffsetX, b.height / 2, "|", nullptr);
	}

	m_blinkTimer += deltaTime;
	if (m_blinkTimer >= 0.4f) {
		m_blinkTimer = 0.0f;
		m_cursorShow = !m_cursorShow;
	}
}

bool Edit::onKeyPress(int keyCode) {
	if (!m_focused) return false;

	switch (keyCode) {
		case VK_RIGHT: m_cursorX = std::min(text.size(), m_cursorX + 1); break;
		case VK_LEFT: m_cursorX = m_cursorX > 0 ? m_cursorX - 1 : 0; break;
		case VK_BACK: {
			if (m_cursorX > 0) {
				m_cursorX = m_cursorX > 0 ? m_cursorX - 1 : 0;
				text.erase(text.begin() + m_cursorX);
				if (onChange) onChange(text);
			}
		} break;
		case VK_DELETE: {
			text.erase(text.begin() + m_cursorX);
			m_cursorX = std::min(text.size(), m_cursorX);
			if (onChange) onChange(text);
		} break;
		case VK_HOME: m_cursorX = 0; break;
		case VK_END: m_cursorX = text.size(); break;
		default: resetCursor(); return false;
	}

	resetCursor();
	return true;
}

void Edit::onMouseDown(int button, int x, int y) {
	if (button == 1) {
		size_t i = 0;
		for (const auto& glyph : m_glyphs) {
			Rect grect = { glyph.x + textPad + m_labelOffset + 4, 0, glyph.maxx - glyph.minx, bounds.height };
			if (grect.hasPoint({ x, y })) {
				m_cursorX = i;
				break;
			}
			i++;
		}
		resetCursor();
	}
}

bool Edit::onType(TCHAR charCode) {
	if (m_focused && ::isprint(int(charCode)) && std::regex_match(std::string(1, char(charCode)), inputFilter)) {
		text.insert(text.begin() + m_cursorX, char(charCode));
		m_cursorX++;

		if (onChange) onChange(text);

		resetCursor();
		return true;
	}
	return false;
}

void Edit::resetCursor() {
	m_cursorShow = true;
	m_blinkTimer = 0.0f;
}
