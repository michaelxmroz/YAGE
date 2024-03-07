#pragma once
#include "RendererVulkan.h"
#include "EngineState.h"

struct UIState
{
	bool m_showMenuBar = true;
	float m_menuBarAlpha = 1.0f;
	bool m_submenuShown = false;
};

class UI
{
public:
	explicit UI(RendererVulkan& renderer);
	void Prepare(EngineData& data);
	void Draw(RendererVulkan& renderer);
	~UI();
private:
	UI(const UI& other) = delete;
	UI& operator= (const UI& other) = delete;

	UIState m_state;
};