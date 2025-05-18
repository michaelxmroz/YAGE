#pragma once
#include "EngineState.h"

class DebuggerUI
{
public:
    void Draw(EngineData& data);

    void Toggle(EngineData& data);
private:
    struct DebuggerUIState
    {
        bool m_showWindow{false};
        int m_stepCount{1};
        int m_selectedMemoryCell{0};
    };

    DebuggerUIState m_state;
};

