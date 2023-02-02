#pragma once

#include "nanovg/nanovg.h"
#include <string>

struct Icon {
	int viewBoxWidth, viewBoxHeight;
	std::string pathData;

	inline void render(NVGcontext* ctx, float ox, float oy, float r, float g, float b, float a = 1.0f, float size = 24.0f) {
		std::string path = pathData;

		auto readChar = [&]() {
			if (path.empty()) return '\0';
			char c = path[0];
			path.erase(0, 1);
			return c;
		};

		auto readNumber = [&]() {
			std::string numStr = "";
			while (!path.empty() && ((path[0] >= '0' && path[0] <= '9') || path[0] == '.' || path[0] == '-')) {
				numStr += readChar();
			}
			return std::stof(numStr);
		};

		auto skipSpaces = [&]() {
			while (!path.empty() && ::isspace(path[0])) {
				readChar();
			}
		};

		auto skipComma = [&]() {
			skipSpaces();
			if (path[0] == ',') readChar();
			skipSpaces();
		};

		nvgSave(ctx);
		nvgTranslate(ctx, ox, oy);
		nvgTranslate(ctx, -size / 2.0f, -size / 2.0f);
		nvgScale(ctx, size / float(viewBoxWidth), size / float(viewBoxHeight));

		nvgBeginPath(ctx);
		while (!path.empty()) {
			char c = readChar();

			if (c == 'M') {
				float x = readNumber(); skipComma();
				float y = readNumber();
				nvgMoveTo(ctx, x, y);
			}
			else if (c == 'L') {
				float x = readNumber(); skipComma();
				float y = readNumber();
				nvgLineTo(ctx, x, y);
			}
			else if (c == 'H') {
				float x = readNumber();
				nvgLineTo(ctx, x, 0.0f);
			}
			else if (c == 'V') {
				float y = readNumber();
				nvgLineTo(ctx, 0.0f, y);
			}
			else if (c == 'Z') {
				nvgClosePath(ctx);
			}
			else if (c == 'C') {
				float x1 = readNumber(); skipComma();
				float y1 = readNumber(); skipComma();
				float x2 = readNumber(); skipComma();
				float y2 = readNumber(); skipComma();
				float x = readNumber(); skipComma();
				float y = readNumber();
				nvgBezierTo(ctx, x1, y1, x2, y2, x, y);
			}
			else if (c == 'Q') {
				float cx = readNumber(); skipComma();
				float cy = readNumber(); skipComma();
				float x = readNumber(); skipComma();
				float y = readNumber();
				nvgQuadTo(ctx, cx, cy, x, y);
			}
		}
		nvgFillColor(ctx, nvgRGBAf(r, g, b, a));
		nvgFill(ctx);

		nvgRestore(ctx);
	}
};

enum Icons {
	icoClose = 0
};

inline static Icon icons[] = {
	{ 24, 24, "M19,6.41L17.59,5L12,10.59L6.41,5L5,6.41L10.59,12L5,17.59L6.41,19L12,13.41L17.59,19L19,17.59L13.41,12L19,6.41Z" }
};
