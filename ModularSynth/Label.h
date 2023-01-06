#pragma once

#include "Control.h"
#include <string>

class Label : public Control {
public:
	void onDraw(NVGcontext* ctx, float deltaTime) override;

	std::string text{ "Label" };
	float fontSize{ 16.0f };
	HorizontalAlignment alignment{ HorizontalAlignment::left };
};
