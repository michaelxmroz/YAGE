#pragma once
#include <thread>

class AudioWin32
{
public:
	void Init();
	void Terminate();
private:

	struct AudioState
	{
		bool m_run;
		bool m_stopped;
	};

	class AudioController
	{
	public:
		void operator()();
		AudioState m_state;
	private:
		void PlayAudioStream();
	};

	AudioController m_audio;
	std::thread m_audioThread;
};

