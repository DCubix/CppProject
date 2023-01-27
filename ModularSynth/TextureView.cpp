#include "TextureView.h"

#define NANOVG_GL3
#include "nanovg/nanovg_gl.h"

constexpr int checkerboardSize = 16;

void TextureView::onDraw(NVGcontext* ctx, float deltaTime) {
	Rect b = bounds;

	if (m_texture != m_textureOld) {
		if (m_image != -1) {
			nvgDeleteImage(ctx, m_image);
			m_image = -1;
		}
		else if (m_textureOld != nullptr && m_texture != nullptr) {
			if (m_textureOld->size()[0] != m_texture->size()[0] || m_textureOld->size()[1] != m_texture->size()[1]) {
				nvgDeleteImage(ctx, m_image);
				m_image = -1;
			}
		}

		if (m_texture) {
			m_image = nvglCreateImageFromHandleGL3(
				ctx,
				m_texture->id(),
				m_texture->size()[0],
				m_texture->size()[1],
				0
			);
		}
	}
	
	//nvgIntersectScissor(ctx, 0, 0, b.width, b.height);
	for (int x = 0; x < b.width; x += checkerboardSize) {
		for (int y = 0; y < b.height; y += checkerboardSize) {
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

	if (!m_texture || m_image == -1) return;

	/*float xform[9];
	nvgCurrentTransform(ctx, xform);*/

	float ratioW = float(bounds.width) / float(m_texture->size()[0]);
	float ratioH = float(bounds.height) / float(m_texture->size()[1]);
	float ratio = ratioW < ratioH ? ratioW : ratioH;
	float nWidth = float(m_texture->size()[0]) * ratio;
	float nHeight = float(m_texture->size()[1]) * ratio;
	float nX = b.width / 2 - nWidth / 2;
	float nY = b.height / 2 - nHeight / 2;

	NVGpaint imgPaint = nvgImagePattern(ctx, nX, nY, nWidth, nHeight, 0.0f, m_image, 1.0f);
	nvgBeginPath(ctx);
	nvgRect(ctx, nX, nY, nWidth, nHeight);
	nvgFillPaint(ctx, imgPaint);
	nvgFill(ctx);
}

void TextureView::setTexture(Texture* texture) {
	m_textureOld = m_texture;
	m_texture = texture;
}
