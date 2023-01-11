#include "Label.h"

void Label::onDraw(NVGcontext* ctx, float deltaTime) {
	Rect b = bounds;

	nvgScissor(ctx, 0, 0, b.width, b.height);

	nvgFontSize(ctx, fontSize);
	nvgFillColor(ctx, nvgRGB(250, 250, 250));
	
	switch (alignment) {
		case HorizontalAlignment::left: nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE); break;
		case HorizontalAlignment::right: nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE); break;
		case HorizontalAlignment::center: nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE); break;
	}

	nvgTextBox(ctx, 0, b.height / 2 + 1.5f, b.width, text.c_str(), nullptr);

	/*nvgBeginPath(ctx);
	nvgRect(ctx, 0, 0, b.width, b.height);
	nvgStrokeColor(ctx, nvgRGB(255, 0, 255));
	nvgStroke(ctx);*/
}
