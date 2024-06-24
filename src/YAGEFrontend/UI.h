#pragma once
#include "RendererVulkan.h"
#include "EngineState.h"
#include "imgui.h"
#include "Tween.h"

struct UIState
{
	struct SubmenuState
	{
		SubmenuState() : m_open(false), m_previousFrame(false) {}
		void Update(bool open)
		{
			m_open = open;
		}

		void EndFrame()
		{
			m_previousFrame = m_open;
		}

		bool HasOpened() const
		{
			return m_open && !m_previousFrame;
		}

		bool HasClosed() const
		{
			return !m_open && m_previousFrame;
		}

		bool IsOpen() const
		{
			return m_open;
		}

		bool m_open = false;
		bool m_previousFrame = false;
	};
	enum class ActiveWindow : uint32_t
	{
		NONE = 0,
		SYSTEM,
		GRAPHICS,
		AUDIO,
		INPUTS
	};

	bool m_showMenuBar = true;
	SubmenuState m_submenuState;
	std::string m_keybindingTitle = "##";
	uint32_t m_keybindingIndex = 0;
	ActiveWindow m_activeWindow = ActiveWindow::NONE;
	uint32_t m_lastMessageIndex = 0;
	bool m_showLogWindow = false;
	ImFont* m_fontSmall = nullptr;
	ImFont* m_fontLarge = nullptr;

	Tween m_menuBarAlphaTween;
	Tween m_messageAlphaTween;

	std::string formatedMessage;
};

class UI
{
public:
	explicit UI(RendererVulkan& renderer);
	void Prepare(EngineData& data, double deltaMs);
	void Draw(RendererVulkan& renderer);
	~UI();
private:
	UI(const UI& other) = delete;
	UI& operator= (const UI& other) = delete;

	UIState m_state;
};