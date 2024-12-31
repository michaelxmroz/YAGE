#pragma once
#include "CppIncludes.h"
#include "Helpers.h"
#include "Memory.h"

namespace Interrupts
{
    enum class Types
    {
        VBlank = 0,
        LCD_STAT = 1,
        Timer = 2,
        Serial = 3,
        Joypad = 4
    };

    void Init(Memory& memory);

    void EnableInterrupt(Types type, Memory& memory);

    void DisableInterrupt(Types type, Memory& memory);

    void RequestInterrupt(Types type, Memory& memory);

    bool HasInterruptRequest(Types type, Memory& memory);

    void ClearInterruptRequest(Types type, Memory& memory);

    bool ShouldHandleInterrupt(Memory& memory);

    bool ShouldHandleInterrupt(Types type, Memory& memory);

    uint16_t GetJumpAddrAndClear(Memory& memory);
}
