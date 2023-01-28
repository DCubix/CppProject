#include <iostream>
#include "ScrollBar.h"


constexpr float sliderHeight = 20.0f;
constexpr float popupHeignt = 18.0f;
constexpr float stepTime = 0.1f;

inline void Rotate(float nx, float ny, float& x, float& y) {
	float resX = nx * x - ny * y;
	float resY = nx * y - ny * x;
	x = resX;
	y = resY;
}


inline void drawArrow(NVGcontext* ctx,  float x, float y, float radius, float dirX, float dirY, float r, float g, float b, float a) {
	

	float x1 = radius * 0.8f;
	float y1 = 0.0f;

	float x2 = -radius * 0.8f;
	float y2 = -radius * 0.8f;

	float x3 = -radius * 0.8f;
	float y3 = +radius * 0.8f;

	Rotate(dirX, dirY, x1, y1);
	Rotate(dirX, dirY, x2, y2);
	Rotate(dirX, dirY, x3, y3);

	x1 += x;
	y1 += y;

	x2 += x;
	y2 += y;

	x3 += x;
	y3 += y;


	nvgBeginPath(ctx);
	nvgMoveTo(ctx, x1, y1);
	nvgLineTo(ctx, x2, y2);
	nvgLineTo(ctx, x3, y3);
	nvgClosePath(ctx);
	nvgFillColor(ctx, nvgRGBAf(r, g, b, a));
	nvgFill(ctx);
}

void ScrollBar::onDraw(NVGcontext* ctx, float deltaTime) {

	//Bit nasty but this'll do
	if(m_decreasing) {
		if(stepTimer <= 0.f) {
			page = std::clamp(page - pageStep, pageMin, pageMax - pageSize);
			stepTimer += stepTime;
		}
		stepTimer -= deltaTime;
	}

	if(m_increasing) {
		if(stepTimer <= 0.f) {
			page = std::clamp(page + pageStep, pageMin, pageMax - pageSize);
			stepTimer += stepTime;
		}
		stepTimer -= deltaTime;
	}

	if (m_dragging) {
		float size = std::min(bounds.width, bounds.height);
		switch(orientation) 
		{
			case SBOrientation::vertical: calculateValue(m_mousePos.y); break;
			case SBOrientation::horizontal: calculateValue(m_mousePos.x); break;
		}
	}

	Rect b = bounds;
	float size = std::min(b.width, b.height);
	Rect sliderRect = {};

	switch(orientation) {
		case SBOrientation::horizontal: sliderRect = { 0, b.height / 2 - int(sliderHeight / 2), b.width, int(sliderHeight) }; break;
		case SBOrientation::vertical: sliderRect = { b.width / 2 - int(sliderHeight / 2), 0, int(sliderHeight), b.height }; break;
	}

	const float radius = (size / 2) - 2;
	

	//Background
	nvgBeginPath(ctx);
	nvgRoundedRect(ctx, sliderRect.x, sliderRect.y, sliderRect.width, sliderRect.height, radius);
	nvgFillColor(ctx, nvgRGBAf(0.0f, 0.0f, 0.0f, 0.6f));
	nvgFill(ctx);


	//Render the buttons
	if(m_active) {
		switch(orientation) {
			case SBOrientation::vertical: 
				if(m_mousePos.y < size) {
					if(m_decreasing) {
						nvgBeginPath(ctx);
						nvgRoundedRect(ctx, 0, 0, size, size, radius);
						nvgFillColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
						nvgFill(ctx);
					}
					else {
						nvgBeginPath(ctx);
						nvgRoundedRect(ctx, 0, 0, size, size, radius);
						nvgStrokeColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
						nvgStroke(ctx);
					}
				} else if(m_mousePos.y > bounds.height - size) {
					if(m_increasing) {
						nvgBeginPath(ctx);
						nvgRoundedRect(ctx, 0, sliderRect.height-size, size, size, radius);
						nvgFillColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
						nvgFill(ctx);
					}
					else {
						nvgBeginPath(ctx);
						nvgRoundedRect(ctx, 0, sliderRect.height-size, size, size, radius);
						nvgStrokeColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
						nvgStroke(ctx);
					}
				}
			break;
			case SBOrientation::horizontal: 
				if(m_mousePos.x < size) {
					if(m_decreasing) {
						nvgBeginPath(ctx);
						nvgRoundedRect(ctx, 0, 0, size, size, radius);
						nvgFillColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
						nvgFill(ctx);
					}
					else {
						nvgBeginPath(ctx);
						nvgRoundedRect(ctx, 0, 0, size, size, radius);
						nvgStrokeColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
						nvgStroke(ctx);
					}
				} else if(m_mousePos.x > bounds.width - size) {
					if(m_increasing) {
						nvgBeginPath(ctx);
						nvgRoundedRect(ctx, sliderRect.width-size, 0, size, size, radius);
						nvgFillColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
						nvgFill(ctx);
					}
					else {
						nvgBeginPath(ctx);
						nvgRoundedRect(ctx, sliderRect.width-size, 0, size, size, radius);
						nvgStrokeColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
						nvgStroke(ctx);
					}
				}

			break;
		}
	}


	nvgBeginPath(ctx);
	
	auto handle = handleRect();
	nvgRoundedRect(ctx, handle.x, handle.y, handle.width, handle.height, radius);
	
	nvgFillColor(ctx, nvgRGBf(1.0f, 1.0f, 1.0f));
	nvgFill(ctx);

	// arrows

	switch(orientation) {
		case SBOrientation::vertical: 
			drawArrow(ctx, size / 2, radius, 4.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, 0.7f);
			drawArrow(ctx, size / 2, b.height - radius, 4.0f, 0.0f, -1.0f, 0.5f, 0.5f, 0.5f, 0.7f);
		break;
		case SBOrientation::horizontal: 
			drawArrow(ctx, radius, size / 2, 4.0f, -1.0f, 0.f, 0.5f, 0.5f, 0.5f, 0.7f);
			drawArrow(ctx, b.width - radius, size / 2, 4.0f, 1.0f, 0.f, 0.5f, 0.5f, 0.5f, 0.7f);		
		break;
	}


}

void ScrollBar::onPostDraw(NVGcontext* ctx, float deltaTime) {
	return;
}

void ScrollBar::onMouseDown(int button, int x, int y) {
	m_mousePos = { x, y };
	if (button == 1) {

		auto handle = handleRect();
		float size = std::min(bounds.width, bounds.height);
		m_grabCoords = { x - handle.x, y - handle.y };
		switch(orientation) 
		{
			case SBOrientation::vertical: 
				if(y < (size))
				{
					m_decreasing = true;
				}
				else if(y > (bounds.height - size)) 
				{
					m_increasing = true;
				}
				else {
					if(handle.hasPoint(Point{ x, y })) 
						m_dragging = true;
				}
			break;
			case SBOrientation::horizontal: 
				if(x < size) 
				{
					m_decreasing = true;
				}
				else if(x > (bounds.width - size)) 
				{
					m_increasing = true;
				}
				else {
					if(handle.hasPoint(Point{ x, y })) 
						m_dragging = true;
				}
			break;
		}


		m_anim.forward(1.0f, 0.0f, 0.3f);
	}
}

void ScrollBar::onMouseUp(int button, int x, int y) {
	m_dragging = false;
	m_increasing = false;
	m_decreasing = false;
	m_mousePos = { x, y };

	m_anim.reverse(0.3f);
}

void ScrollBar::onMouseMove(int x, int y, int dx, int dy) {

	m_mousePos = { x, y };

}

void ScrollBar::onMouseEnter() 
{
	m_active = true;
}

void ScrollBar::onMouseLeave() 
{
	m_active = false;
}

void ScrollBar::calculateValue(int val) {

	Rect b = bounds;
	Rect area = handleArea();

	
	float size = std::min(b.width, b.height);
	float range = pageMax - pageMin;


	float newPage = val;
	switch(orientation) {
		case SBOrientation::vertical: 
			newPage = pageMin + (val - area.y - m_grabCoords.y + 1) * range / float(area.height);
		break;
		case SBOrientation::horizontal: 
			newPage = pageMin + (val - area.x - m_grabCoords.x + 1) * range / float(area.width);
		break;
	}

	newPage = std::clamp(newPage, pageMin, pageMax - pageSize);
	newPage = int(newPage / pageStep + 0.5f) * pageStep;

	if (page != newPage) 
	{
		page = newPage;
		if (onChange) onChange(page);
	}
}

Rect ScrollBar::handleArea() {
	float size = std::min(bounds.width, bounds.height);
	Rect result = {};
	switch(orientation) {
		case SBOrientation::vertical: 
			result.x = 0;
			result.y = size;
			result.width = size;
			result.height = bounds.height - size*2;
		break;
		case SBOrientation::horizontal: 
			result.x = size;
			result.y = 0;
			result.width = bounds.width - size*2;
			result.height = size;
		break;
	}

	return result;
}

Rect ScrollBar::handleRect() {

	float contentSize = pageMax - pageMin;
	float value = (page - pageMin) / contentSize;
	float pageScale = pageSize / contentSize;
	
	Rect subRect = handleArea();

	Rect result = {};
	

	switch(orientation) {
		case SBOrientation::vertical: 
			result.y = subRect.y + float(subRect.height) * value;
			result.width = subRect.width;
			result.height = (float(subRect.height) * pageScale + 0.5f);
		break;
		case SBOrientation::horizontal: 
			result.x = subRect.x + float(subRect.width) * value;
			result.width = (float(subRect.width) * pageScale + 0.5f);
			result.height = subRect.height;
		break;
	}

	return result;

}

Rect ScrollBar::negativeQuickScrollArea() {

	float size = std::min(bounds.width, bounds.height);
	Rect result = {};
	Rect handle = handleRect();

	switch(orientation) {
		case SBOrientation::vertical: 
			result.y = size;
			result.width = size;
			result.height = handle.y - size;
		break;
		case SBOrientation::horizontal: 
			result.x = size;
			result.height = size;
			result.width = handle.x - size;
		break;
	}

	return result;

}

Rect ScrollBar::positiveQuickScrollArea() {

	float size = std::min(bounds.width, bounds.height);
	Rect result = {};
	Rect handle = handleRect();

	switch(orientation) {
		case SBOrientation::vertical: 
			result.y = handle.y+handle.height;
			result.width = size;
			result.height = bounds.height - result.y - size;
		break;
		case SBOrientation::horizontal: 
			result.x = handle.x+handle.width;
			result.height = size;
			result.width = bounds.width - result.x - size;
		break;
	}

	return result;
}

