#pragma once
#include <cstdint>
#include "Helpers.h"

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

    void EnableInterrupt(Types type, uint8_t* memory);

    void DisableInterrupt(Types type, uint8_t* memory);

    void RequestInterrupt(Types type, uint8_t* memory);

    void ClearInterruptRequest(Types type, uint8_t* memory);

    bool ShouldHandleInterrupt(uint8_t* memory);

    bool ShouldHandleInterrupt(Types type, uint8_t* memory);

    uint16_t GetJumpAddrAndClear(uint8_t* memory);
}
