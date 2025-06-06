#pragma once
#include "EngineState.h"

class DebuggerUI
{
public:
    void Draw(DebuggerState& data);

    void Toggle(DebuggerState& data);
private:
    struct DebuggerUIState
    {
        bool m_showWindow{false};
        int m_stepCount{1};
        int m_selectedMemoryCell{-1};
        int m_memoryFirstVisibleAddr{-1};
        int m_hoveredAddr{-1};
    };

    DebuggerUIState m_state{};
};

