#pragma once

#include <Windows.h>

#include <vector>
#include <algorithm>
#include <cassert>
#include <memory>

template <typename EventType>
class Listener {
public:
	virtual void onEventReceived(const EventType& e) = 0;
};

template <typename EventType>
class EventSystem {
public:
	using ListenerPtr = std::shared_ptr<Listener<EventType>>;

	EventSystem() = default;
	~EventSystem() = default;

	void addListener(const ListenerPtr& listener) {
		assert(listener != nullptr);
		m_listeners.push_back(listener);
	}

	void removeListener(const ListenerPtr& listener) {
		assert(listener != nullptr);
		auto pos = std::find_if(m_listeners.begin(), m_listeners.end(), [&](ListenerPtr& el) { return el.get() == listener.get(); });
		if (pos != m_listeners.end()) {
			m_listeners.erase(pos);
		}
	}

	void notify(const EventType& e) {
		for (ListenerPtr& listener : m_listeners) {
			if (listener.use_count() == 0) continue;
			listener->onEventReceived(e);
		}
	}

private:
	std::vector<ListenerPtr> m_listeners;
};

/**   EVENT TYPES   */
struct Point { int x, y; };

struct MouseMotionEvent {
	Point screenPosition;
	int deltaX, deltaY;
};

using MouseMotionListener = Listener<MouseMotionEvent>;
using MouseMotionEventSystem = EventSystem<MouseMotionEvent>;

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
using MouseButtonEventSystem = EventSystem<MouseButtonEvent>;


struct KeyboardEvent {
	int keyCode;
	TCHAR keyChar;
	ButtonState state;
};


using KeyboardListener = Listener<KeyboardEvent>;
using KeyboardEventSystem = EventSystem<KeyboardEvent>;
