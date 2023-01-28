#include <iostream>
#include "ScrollBar.h"


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
	update(deltaTime);

	Rect b = bounds;
	float size = std::min(b.width, b.height);
	Rect sliderRect = {};

	switch(orientation) {
		case SBOrientation::horizontal: sliderRect = { 0, b.height / 2 - int(size / 2), b.width, int(size) }; break;
		case SBOrientation::vertical: sliderRect = { b.width / 2 - int(size / 2), 0, int(size), b.height }; break;
	}

	const float radius = (size / 2) - 2;
	
	bool vertical = orientation == SBOrientation::vertical;
	Rect posQuickScroll = positiveQuickScrollArea();
	Rect negQuickScroll = negativeQuickScrollArea();

	//Background
	nvgBeginPath(ctx);
	nvgRoundedRect(ctx, sliderRect.x, sliderRect.y, sliderRect.width, sliderRect.height, radius);
	nvgFillColor(ctx, nvgRGBAf(0.0f, 0.0f, 0.0f, 0.6f));
	nvgFill(ctx);

	Rect* rct = nullptr;
	bool visualizeQuickScroll = false;
	if(posQuickScroll.hasPoint(m_mousePos)) {
		rct = &posQuickScroll;
		visualizeQuickScroll = true;
	}
	else if(negQuickScroll.hasPoint(m_mousePos)) {
		rct = &negQuickScroll;
		visualizeQuickScroll = true;
	}

	if(visualizeQuickScroll) {
		
		int w = rct->width;
		int h = rct->height;
		if(vertical) w /= 2; else h /= 2;

		nvgBeginPath(ctx);
		nvgRect(ctx, rct->x, rct->y, w, h);
		auto paint = nvgLinearGradient(
			ctx, 
			0, 
			0, 
			vertical ? w : 0, 
			vertical ? 0 : h, 
			nvgRGBAf(1.f, 1.f, 1.f, 0.f),
			nvgRGBAf(1.f, 1.f, 1.f, 0.25f)
		);
		nvgFillPaint(ctx, paint);
		nvgFill(ctx);

		nvgBeginPath(ctx);
		nvgRect(ctx, rct->x + (vertical ? w : 0), rct->y + (vertical ? 0 : h), w, h);
		paint = nvgLinearGradient(
			ctx, 
			vertical ? w : 0, 
			vertical ? 0 : h, 
			vertical ? w * 2 : 0, 
			vertical ? 0 : h * 2, 
			nvgRGBAf(1.f, 1.f, 1.f, 0.25f),
			nvgRGBAf(1.f, 1.f, 1.f, 0.f)
		);
		nvgFillPaint(ctx, paint);
		nvgFill(ctx);
	}


	//Render the buttons
	if(m_active) {
		switch(orientation) {
			case SBOrientation::vertical: 
				if(m_mousePos.y < size) {
					if(m_decreasing) {
						nvgBeginPath(ctx);
						nvgRoundedRectVarying(ctx, 0, 0, size, size, radius, radius, 0, 0);
						nvgFillColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
						nvgFill(ctx);
					}
					else {
						nvgBeginPath(ctx);
						nvgRoundedRectVarying(ctx, 0, 0, size, size, radius, radius, 0, 0);
						nvgStrokeColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 0.75f));
						nvgStroke(ctx);
					}
				} else if(m_mousePos.y > bounds.height - size) {
					if(m_increasing) {
						nvgBeginPath(ctx);
						nvgRoundedRectVarying(ctx, 0, sliderRect.height-size, size, size, 0, 0, radius, radius);
						nvgFillColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
						nvgFill(ctx);
					}
					else {
						nvgBeginPath(ctx);
						nvgRoundedRectVarying(ctx, 0, sliderRect.height-size, size, size, 0, 0, radius, radius);
						nvgStrokeColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 0.75f));
						nvgStroke(ctx);
					}
				}
			break;
			case SBOrientation::horizontal: 
				if(m_mousePos.x < size) {
					if(m_decreasing) {
						nvgBeginPath(ctx);
						nvgRoundedRectVarying(ctx, 0, 0, size, size, radius, 0.0f, 0.0f, radius);
						nvgFillColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
						nvgFill(ctx);
					}
					else {
						nvgBeginPath(ctx);
						nvgRoundedRectVarying(ctx, 0, 0, size, size, radius, 0.0f, 0.0f, radius);
						nvgStrokeColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 0.75f));
						nvgStroke(ctx);
					}
				} else if(m_mousePos.x > bounds.width - size) {
					if(m_increasing) {
						nvgBeginPath(ctx);
						nvgRoundedRectVarying(ctx, bounds.width - size, 0, size, size, 0.0f, radius, radius, 0.0f);
						nvgFillColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
						nvgFill(ctx);
					}
					else {
						nvgBeginPath(ctx);
						nvgRoundedRectVarying(ctx, bounds.width - size, 0, size, size, 0.0f, radius, radius, 0.0f);
						nvgStrokeColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 0.75f));
						nvgStroke(ctx);
					}
				}

			break;
		}
	}


	nvgBeginPath(ctx);
	
	float t = (page - pageMin) / (pageMax - pageMin - pageSize);
	t = (t * t * (3.0f - 2.0f * t));

	float lowRadius = (1.0f - t) * radius;
	float highRadius = t * radius;

	auto handle = handleRect();
	if(vertical)
		nvgRoundedRectVarying(ctx, handle.x, handle.y, handle.width, handle.height, highRadius, highRadius, lowRadius, lowRadius);
	else
		nvgRoundedRectVarying(ctx, handle.x, handle.y, handle.width, handle.height, highRadius, lowRadius, lowRadius, highRadius);
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
		auto positiveQuickScroll = positiveQuickScrollArea();
		auto negativeQuickScroll = negativeQuickScrollArea();
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
					if(handle.hasPoint(m_mousePos)) 
						m_dragging = true;
					else if(positiveQuickScroll.hasPoint(m_mousePos)) {
						m_increasing = true;
						m_quickMode = true;
					} else if(negativeQuickScroll.hasPoint(m_mousePos)) {
						m_decreasing = true;
						m_quickMode = true;
					}
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
					if(handle.hasPoint(m_mousePos)) 
						m_dragging = true;
					else if(positiveQuickScroll.hasPoint(m_mousePos)) {
						m_increasing = true;
						m_quickMode = true;
					} else if(negativeQuickScroll.hasPoint(m_mousePos)) {
						m_decreasing = true;
						m_quickMode = true;
					}
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

void ScrollBar::update(float deltaTime) {

	auto handle = handleRect();

	if(m_quickMode && handle.hasPoint(m_mousePos)) {
		m_quickMode = false;
		m_decreasing = false;
		m_increasing = false;
	}

	if(m_decreasing) {

		if(m_quickMode) {
	
			if(stepTimer <= 0.0f) {
				page = std::clamp(page - pageSize, pageMin, pageMax - pageSize);
				stepTimer += stepTime;
			}
		}
		else {
			if(stepTimer <= 0.0f) {
				page = std::clamp(page - pageStep, pageMin, pageMax - pageSize);
				stepTimer += stepTime;
			}
		}
		stepTimer -= deltaTime;
	}

	if(m_increasing) {
		if(m_quickMode) {
			if(stepTimer <= 0.0f && !handle.hasPoint(m_mousePos)) {
				page = std::clamp(page + pageSize, pageMin, pageMax - pageSize);
				stepTimer += stepTime;
			}
		}
		else {
			if(stepTimer <= 0.0f) {
				page = std::clamp(page + pageStep, pageMin, pageMax - pageSize);
				stepTimer += stepTime;
			}
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

