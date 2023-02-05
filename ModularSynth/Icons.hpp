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

		int lastWinding = NVG_CCW;
		while (!path.empty()) {
			char c = readChar();
			skipSpaces();

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
			else if (c == 'K') {
				lastWinding = lastWinding == NVG_CCW ? NVG_CW : NVG_CCW;
				nvgPathWinding(ctx, lastWinding);
			}
		}

		nvgStrokeWidth(ctx, 0.0f);
		nvgStrokeColor(ctx, nvgRGBAf(r, g, b, 0.0f));
		nvgFillColor(ctx, nvgRGBAf(r, g, b, a));
		nvgFill(ctx);

		nvgRestore(ctx);
	}

};

enum Icons {
	icoClose = 1,
	icoFolderOpen,
	icoSave
};

// use this tool to fix the icons https://yqnn.github.io/svg-path-editor/
inline static Icon icons[] = {
	{ 0, 0, "" },
	{ 24, 24, "M19,6.41L17.59,5L12,10.59L6.41,5L5,6.41L10.59,12L5,17.59L6.41,19L12,13.41L17.59,19L19,17.59L13.41,12L19,6.41Z" },
	{ 24, 24, "M 19 20 L 4 20 C 2.89 20 2 19.1 2 18 L 2 6 C 2 4.89 2.89 4 4 4 L 10 4 L 12 6 L 19 6 C 19.6667 6.6667 20.3333 7.3333 21 8 L 21 8 L 4 8 L 4 18 L 6.14 10 L 23.21 10 L 20.93 18.5 C 20.7 19.37 19.92 20 19 20 Z" },
	{ 24, 24, "M 15 9 L 5 9 L 5 5 L 15 5 M 12 19 C 10.4 19 9 17.5 9 16 C 9 14.4 10.4 13 12 13 C 13.6 13 15 14.4 15 16 C 15 17.6 13.6 19 12 19 M 17 3 L 5 3 C 3.89 3 3 3.9 3 5 L 3 19 C 3 20 4 21 5 21 L 19 21 C 20 21 21 20 21 19 L 21 7 L 17 3 K Z" }
};
