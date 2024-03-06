#pragma once

#include "EngineState.h"
#include <Emulator.h>
#include "Logging.h"
#include "Clock.h"
#include "FileParser.h"
#include <iostream>
#include "RendererVulkan.h"
#include "Audio.h"
#include "Input.h"
#include "UI.h"


#define PERSISTENT_MEMORY_FILE_ENDING "sav"
#define SAVE_STATE_FILE_ENDING "ssf"

class EngineController
{
public:

    explicit EngineController(EngineData state);
	void Run();
    ~EngineController();

private:

    static void SavePersistentMemory(const void* data, uint32_t size);

    EngineController(const EngineController& other) = delete;
    EngineController& operator=(const EngineController& other) = delete;

    void CreateEmulator(const std::vector<char>& bootromBlob, const std::vector<char>& romBlob, const std::vector<char>& ramBlob);
    void CleanupEmulator();
    void RunEmulatorLoop();

    static std::string s_persistentMemoryPath;

    EngineData m_data;
    Renderer* m_renderer;
    Audio* m_audio;
    UI* m_UI;
    InputHandler* m_inputHandler;
    Emulator* m_emulator;
    const double m_preferredFrameTime = 1000.0 / EmulatorConstants::PREFERRED_REFRESH_RATE;

};