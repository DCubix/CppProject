#pragma once

#include "Control.h"
#include "Texture.h"

class TextureView : public Control {
public:
	void onDraw(NVGcontext* ctx, float deltaTime) override;

	void setTexture(Texture* texture);

private:
	int m_image{ -1 };
	Texture* m_texture{ nullptr }, *m_textureOld{ nullptr };
};
