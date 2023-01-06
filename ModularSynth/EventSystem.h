#pragma once

#include <Windows.h>

#include <vector>
#include <algorithm>
#include <cassert>

template <typename EventType>
class Listener {
public:
	virtual void onEventReceived(const EventType& e) = 0;
};

template <typename EventType>
class EventSystemBase {
public:
	EventSystemBase() = default;
	~EventSystemBase() = default;

	void addListener(Listener<EventType>* listener) {
		assert(listener != nullptr);
		m_listeners.push_back(listener);
	}

	void removeListener(Listener<EventType>* listener) {
		assert(listener != nullptr);
		auto pos = std::find(m_listeners.begin(), m_listeners.end(), listener);
		if (pos != m_listeners.end()) {
			m_listeners.erase(pos);
		}
	}

	void notify(const EventType& e) {
		for (auto&& listener : m_listeners) {
			listener->onEventReceived(e);
		}
	}

private:
	std::vector<Listener<EventType>*> m_listeners;
};

/**   EVENT TYPES   */
struct Point { int x, y; };

struct MouseMotionEvent {
	Point screenPosition;
	int deltaX, deltaY;
};

using MouseMotionListener = Listener<MouseMotionEvent>;
using MouseMotionEventSystem = EventSystemBase<MouseMotionEvent>;

enum ButtonState {
	pressed = 0,
	released
};

struct MouseButtonEvent {
	Point screenPosition;
	int button;
	ButtonState state;
};

using MouseButtonListener = Listener<MouseButtonEvent>;
using MouseButtonEventSystem = EventSystemBase<MouseButtonEvent>;


struct KeyboardEvent {
	int keyCode;
	TCHAR keyChar;
	ButtonState state;
};


using KeyboardListener = Listener<KeyboardEvent>;
using KeyboardEventSystem = EventSystemBase<KeyboardEvent>;
