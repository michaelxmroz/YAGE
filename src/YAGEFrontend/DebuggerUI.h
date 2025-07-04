#pragma once
#include "EngineState.h"

#if defined (_DEBUG)
#define DEBUGGER
#endif


class DebuggerUI
{
public:

    void Draw(DebuggerState& data, Emulator* emulator);

    void Toggle(DebuggerState& data);
#if defined (DEBUGGER)
private:
    struct DebuggerUIState
    {
        bool m_showWindow{false};
        int m_stepCount{1};
        int m_selectedMemoryCell{-1};
        int m_memoryFirstVisibleAddr{-1};
        int m_hoveredAddr{-1};
        float m_memoryScrollTarget{-1.0f};
        
        // Unified breakpoint input
        int m_selectedBreakpointType{0}; // Index into breakpoint type dropdown
        char m_unifiedBreakpointInput[16]{""};
        bool m_breakpointInputError{false};
        char m_breakpointErrorMessage[64]{""};
    };

    DebuggerUIState m_state{};
#endif
};

