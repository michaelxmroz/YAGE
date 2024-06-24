#include "Tween.h"
#include "Logger.h"

namespace
{
	float lerp(float a, float b, float t)
	{
		return a + t * (b - a);
	}
}

Tween::Tween() :
	m_begin(0.0f),
	m_end(1.0f),
	m_duration(1000.0f),
	m_falloffStart(0.0f),
	m_normalizingFactor(1.0f)
{}

Tween::Tween(float begin, float end, float durationMs, float falloffStart) :
	m_begin(begin),
	m_end(end),
	m_duration(1.0f / durationMs),
	m_falloffStart(falloffStart / durationMs),
	m_normalizingFactor(1.0f / (1.0f - m_falloffStart))
{}

float Tween::Update(float delta, float invert)
{
	if (invert)
	{
		delta *= -1.0f;
	}

	m_progress = std::min(std::max(m_progress + delta * m_duration, 0.0f), 1.0f);

	float t = std::min(std::max(m_progress - m_falloffStart, 0.0f) * m_normalizingFactor, 1.0f);

	return lerp(m_begin, m_end, t);
}

void Tween::Reset()
{
	m_progress = 0.0f;
}

bool Tween::HasFinished()
{
	return m_progress >= 1.0f;
}
