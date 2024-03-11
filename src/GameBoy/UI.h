#pragma once
#include "RendererVulkan.h"
#include "EngineState.h"

struct UIState
{
	enum class ActiveWindow : uint32_t
	{
		NONE = 0,
		GRAPHICS,
		AUDIO
	};

	bool m_showMenuBar = true;
	float m_menuBarAlpha = 1.0f;
	bool m_submenuShown = false;
	ActiveWindow m_activeWindow = ActiveWindow::NONE;
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