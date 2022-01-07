#include "Interrupts.h"

#define INTERRUPT_ENABLE_REGISTER 0xFFFF
#define INTERRUPT_FLAG_REGISTER 0xFF0F

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


void Interrupts::EnableInterrupt(Types type, Memory& memory)
{
    memory.Write(INTERRUPT_ENABLE_REGISTER) |= (1 << static_cast<uint8_t>(type));
}

void Interrupts::DisableInterrupt(Types type, Memory& memory)
{
    memory.Write(INTERRUPT_ENABLE_REGISTER) &= ~(1 << static_cast<uint8_t>(type));
}

void Interrupts::RequestInterrupt(Types type, Memory& memory)
{
    memory.Write(INTERRUPT_FLAG_REGISTER) |= (1 << static_cast<uint8_t>(type));
}

void Interrupts::ClearInterruptRequest(Types type, Memory& memory)
{
    memory.Write(INTERRUPT_FLAG_REGISTER) &= ~(1 << static_cast<uint8_t>(type));
}

bool Interrupts::ShouldHandleInterrupt(Memory& memory)
{
    return (memory[INTERRUPT_ENABLE_REGISTER] & memory[INTERRUPT_FLAG_REGISTER]) > 0;
}

bool Interrupts::ShouldHandleInterrupt(Types type, Memory& memory)
{
    return (memory[INTERRUPT_ENABLE_REGISTER] & memory[INTERRUPT_FLAG_REGISTER]) == (1 << static_cast<uint8_t>(type));
}

uint16_t Interrupts::GetJumpAddrAndClear(Memory& memory)
{
    uint8_t interrupts = memory[INTERRUPT_ENABLE_REGISTER] & memory[INTERRUPT_FLAG_REGISTER];

    int index = Helpers::GetFirstSetBit(interrupts);
    Types type = static_cast<Types>(index);
    ClearInterruptRequest(type, memory);
    return JUMP_ADDRESSES[index];
}
}