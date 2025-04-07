#include "Interrupts.h"

#define INTERRUPT_ENABLE_REGISTER 0xFFFF
#define INTERRUPT_FLAG_REGISTER 0xFF0F

#define INTERRUPT_REGISTER_MASK 0x1F

const uint16_t JUMP_ADDRESSES[5]
{
    0x0040,
    0x0048,
    0x0050,
    0x0058,
    0x0060
};

namespace Interrupts
{
    void Init(Memory& memory)
    {
        memory.Write(INTERRUPT_ENABLE_REGISTER, 0x0);
        memory.Write(INTERRUPT_FLAG_REGISTER, 0xE1);

        memory.AddIOUnusedBitsOverride(INTERRUPT_ENABLE_REGISTER, 0b11100000);
        memory.AddIOUnusedBitsOverride(INTERRUPT_FLAG_REGISTER, 0b11100000);
    }

    void EnableInterrupt(Types type, Memory& memory)
    {
        memory.Write(INTERRUPT_ENABLE_REGISTER, memory[INTERRUPT_ENABLE_REGISTER] | (1 << static_cast<uint8_t>(type)));
    }

    void DisableInterrupt(Types type, Memory& memory)
    {
        memory.Write(INTERRUPT_ENABLE_REGISTER, memory[INTERRUPT_ENABLE_REGISTER] & ~(1 << static_cast<uint8_t>(type)));
    }

    void RequestInterrupt(Types type, Memory& memory)
    {
        memory.Write(INTERRUPT_FLAG_REGISTER, memory[INTERRUPT_FLAG_REGISTER] | (1 << static_cast<uint8_t>(type)));
    }

    bool HasInterruptRequest(Types type, Memory& memory)
    {
        return (memory[INTERRUPT_FLAG_REGISTER] & (1 << static_cast<uint8_t>(type))) > 0;
    }

    void ClearInterruptRequest(Types type, Memory& memory)
    {
        memory.Write(INTERRUPT_FLAG_REGISTER, memory[INTERRUPT_FLAG_REGISTER] & ~(1 << static_cast<uint8_t>(type)));
    }

    bool ShouldHandleInterrupt(Memory& memory)
    {
        return (memory[INTERRUPT_ENABLE_REGISTER] & memory[INTERRUPT_FLAG_REGISTER] & INTERRUPT_REGISTER_MASK) > 0;
    }

    bool ShouldHandleInterrupt(Types type, Memory& memory)
    {
        return (memory[INTERRUPT_ENABLE_REGISTER] & memory[INTERRUPT_FLAG_REGISTER] & INTERRUPT_REGISTER_MASK) == (1 << static_cast<uint8_t>(type));
    }

    uint16_t GetJumpAddrAndClear(Memory& memory)
    {
        uint8_t interrupts = memory[INTERRUPT_ENABLE_REGISTER] & memory[INTERRUPT_FLAG_REGISTER] & INTERRUPT_REGISTER_MASK;

        int index = Helpers::GetFirstSetBit(interrupts);
        if( index < 0)
        {
            return 0x00;
        }
        Types type = static_cast<Types>(index);
        ClearInterruptRequest(type, memory);
        return JUMP_ADDRESSES[index];
    }
}