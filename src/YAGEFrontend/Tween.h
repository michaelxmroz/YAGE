#pragma once
#include <utility>

class Tween
{
public:
	Tween();
	Tween(float begin, float end, float durationMs, float falloffStart);

	float Update(float delta, float invert = false);
	void Reset();
	bool HasFinished();

private:
	float m_begin;
	float m_end;
	float m_duration;
	float m_falloffStart;
	float m_normalizingFactor;

	float m_progress;
};
