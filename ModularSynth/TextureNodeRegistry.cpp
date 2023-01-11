#include "TextureNodeRegistry.h"

#define NANOVG_GL3
#include "nanovg/nanovg_gl.h"

constexpr int checkerboardSize = 12;

void VisualTextureNode::onExtraDraw(NVGcontext* ctx, float dt) {
	if (!m_node) return;

	if (image == -1) {
		GraphicsNode* gn = dynamic_cast<GraphicsNode*>(m_node);
		image = nvglCreateImageFromHandleGL3(ctx, gn->textureID(), gn->outputWidth, gn->outputHeight, 0);
	}

	Dimension sz = extraSize();

	nvgIntersectScissor(ctx, 0, 0, sz.width, sz.height);
	for (int x = 0; x < sz.width; x += checkerboardSize) {
		for (int y = 0; y < sz.height; y += checkerboardSize) {
			nvgBeginPath(ctx);
			nvgRect(ctx, x, y, checkerboardSize, checkerboardSize);

			int j = x / checkerboardSize;
			int i = y / checkerboardSize;

			if ((i % 2 == 0 && j % 2 == 0) || (i % 2 != 0 && j % 2 != 0))
				nvgFillColor(ctx, nvgRGB(110, 110, 110));
			else
				nvgFillColor(ctx, nvgRGB(70, 70, 70));
			
			nvgFill(ctx);
		}
	}

	float xform[9];
	nvgCurrentTransform(ctx, xform);
	float tx = xform[2];
	float ty = xform[5];

	auto gap = m_size.height - sz.height;

	nvgSave(ctx);
	nvgResetTransform(ctx);
	NVGpaint imgPaint = nvgImagePattern(ctx, 0.0f, 0.0f, sz.width, sz.height, 0.0f, image, 1.0f);
	nvgRestore(ctx);

	nvgBeginPath(ctx);
	nvgRect(ctx, 0.0f, 0.0f, sz.width, sz.height);
	nvgFillPaint(ctx, imgPaint);
	nvgFill(ctx);

	nvgResetScissor(ctx);
}
