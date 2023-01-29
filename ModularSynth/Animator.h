#pragma once

#include <functional>
#include <algorithm>

#define PI 3.141592654f

using Curve = std::function<float(float)>;

enum AnimatorState {
	idling = 0,
	playing,
	finished
};

template <typename T = float>
class Animator {
public:

	Animator() = default;

	T value(const Curve& curve, float deltaTime) {
		switch (m_state) {
			case AnimatorState::playing: {
				float t = m_time / m_duration;
				if (curve) {
					t = curve(t);
				}
				m_value = std::lerp(m_previousTarget, m_target, t);

				if (m_time < m_duration) {
					m_time += deltaTime;
				}
				else {
					m_value = m_target;
					m_time = m_duration;
					m_state = AnimatorState::finished;
				}
			} break;
			default: break;
		}

		return m_value;
	}

	void target(T value, float duration = 0.5f) {
		m_previousTarget = m_value;
		m_value = m_previousTarget;
		m_target = value;
		m_time = 0.0f;
		m_duration = duration;
		m_state = AnimatorState::playing;
	}

	const AnimatorState& state() const { return m_state; }

private:
	float m_time{ 0.0f }, m_duration{ 0.5f }; // seconds

	T m_value{ 0 }, m_target{ 0 }, m_previousTarget{ 0 };
	AnimatorState m_state{ AnimatorState::idling };
};

class Curves {
public:
	static float linear(float t) { return t; }
	static float easeInQuad(float t) { return t * t; }
	static float easeInCubic(float t) { return t * t * t; }
	static float easeInBack(float t) {
		const float c1 = 1.70158f;
		const float c3 = c1 + 1.0f;
		return c3 * easeInCubic(t) - c1 * easeInQuad(t);
	}
	static float easeInElastic(float t) {
		const float c4 = (2.0f * PI) / 3.0f;
		if (t <= 1e-5f) return 0.0f;
		if (t >= 1.0f) return 1.0f;
		return -::powf(2.0f, 10.0f * t - 10.0f) * ::sinf((t * 10.0f - 10.75f) * c4);
	}
	static float easeOutQuad(float t) {
		float nt = 1.0f - t;
		return 1.0f - nt * nt;
	}
	static float easeOutCubic(float t) {
		float nt = 1.0f - t;
		return 1.0f - nt * nt * nt;
	}
	static float easeOutBack(float t) {
		const float c1 = 1.70158f;
		const float c3 = c1 + 1.0f;
		return 1.0f + c3 * ::powf(t - 1.0f, 3.0f) + c1 * ::powf(t - 1.0f, 2.0f);
	}
	static float easeOutElastic(float t) {
		const float c4 = (2.0f * PI) / 3.0f;
		if (t <= 1e-5f) return 0.0f;
		if (t >= 1.0f) return 1.0f;
		return ::powf(2.0f, -10.0f * t) * ::sinf((t * 10.0f - 0.75f) * c4) + 1.0f;
	}
	static float easeInOutQuad(float t) {
		return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
	}
	static float easeInOutCubic(float t) {
		float k = 2.0f * t - 2.0f;
		return t < 0.5f ? 4.0f * t * t * t : (t - 1.0f) * k * k + 1.0f;
	}
	static float easeInOutBack(float t) {
		float s = 1.70158f;
		if ((t /= 0.5f) < 1.0f) return 1.0f / 2.0f * (t * t * (((s *= (1.525f)) + 1.0f) * t - s));
		return 1.0f / 2.0f * ((t -= 2.0f) * t * (((s *= (1.525f)) + 1.0f) * t + s) + 2.0f);
	}
	static float easeInOutElastic(float t) {
		float s = 1.70158f;
		float p = 0;
		float a = 1;
		if (t <= 1e-5f) return 0.0f;
		if ((t /= 0.5f) >= 2.0f - 1e-5f) return 1.0f;
		if (!p) p = (0.3f * 1.5f);
		if (a < 1.0f) {
			a = 1.0f;
			s = p / 4.0f;
		}
		else s = p / (2.0f * PI) * ::asinf(1.0f / a);
		if (t < 1.0f)
			return -0.5f * (a * ::powf(2.0f, 10.0f * (t -= 1.0f)) * ::sinf((t - s) * (2.0f * PI) / p));
		return a * ::powf(2.0f, -10.0f * (t -= 1.0f)) * ::sinf((t - s) * (2.0f * PI) / p) * 0.5f + 1.0f;
	}
};
