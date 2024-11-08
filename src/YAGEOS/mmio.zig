const std = @import("std");

pub const MMIO_BASE: u32 = 0xFE000000;

pub const Registers = enum (u32)
{
    // The offsets for reach register.
    const GPIO_BASE = 0x200000;
    // The base address for UART.
    const UART0_BASE = (GPIO_BASE + 0x1000);
    const MBOX_BASE    = 0xFF800000;
 
    // Mode selector register for GPIO regs 13-19
    GPFSEL1 = (GPIO_BASE + 0x04),

    GPIO_PUP_PDN_CNTRL_REG0 = (GPIO_BASE + 0xe4),

    // The offsets for reach register for the UART.
    UART0_DR     = (UART0_BASE + 0x00), // Data Register
    UART0_RSRECR = (UART0_BASE + 0x04), // Receive status register/error clear register
    UART0_FR     = (UART0_BASE + 0x18), // Flag Register
    UART0_ILPR   = (UART0_BASE + 0x20), // Unused
    UART0_IBRD   = (UART0_BASE + 0x24), // Integer baud rate register
    UART0_FBRD   = (UART0_BASE + 0x28), // Fractional baud rate register
    UART0_LCRH   = (UART0_BASE + 0x2C), // Line control register
    UART0_CR     = (UART0_BASE + 0x30), // Control Register
    UART0_IFLS   = (UART0_BASE + 0x34), // Interrupt FOFO level select register
    UART0_IMSC   = (UART0_BASE + 0x38), // Interrupt mask set/clear register
    UART0_RIS    = (UART0_BASE + 0x3C), // Raw interrupt status register
    UART0_MIS    = (UART0_BASE + 0x40), // Masked interrupt status register
    UART0_ICR    = (UART0_BASE + 0x44), // Interrupt Clear Register
    UART0_DMACR  = (UART0_BASE + 0x48), // DMA Control Register
    UART0_ITCR   = (UART0_BASE + 0x80), // Test control register
    UART0_ITIP   = (UART0_BASE + 0x84), // Test control register
    UART0_ITOP   = (UART0_BASE + 0x88), // Test control register
    UART0_TDR    = (UART0_BASE + 0x8C), // Test data register
};

pub fn printu64(val : u64) void
{
    var n: u6 = 0;
    var shiftVal = val;
    while (n < 64) 
    {
        const shiftMask = 0x1;
        shiftVal = (val >> (63 - n));
        const lastBit = shiftVal & shiftMask;
        if( lastBit == 1)
        {
            uartSend('1');
        }
        else
        {
            uartSend('0');
        }
        n += 1;
    }
    uartSend('\n');
}


pub fn mmioWrite(reg : Registers, data : u32) void
{
    const ptr: *volatile u32 = @ptrFromInt(MMIO_BASE + @intFromEnum(reg));
    ptr.* = data;
}

pub fn mmioRead(reg : Registers) u32
{
    const ptr: *volatile u32 = @ptrFromInt(MMIO_BASE + @intFromEnum(reg));
    return ptr.*;
}


pub fn uartSend(c: u8) void 
{
    while ((mmioRead(Registers.UART0_FR) & (1 << 5)) != 0) {}
    mmioWrite(Registers.UART0_DR, c);
}

pub fn uartSendString(s: []const u8) void 
{
    for (s) |c| 
    {
        uartSend(c);
    }
}

pub fn uartInit() void
{
    // Disable UART0 by clearing the UART Control Register (CR).
    mmioWrite(Registers.UART0_CR, 0x00000000);

    // Clear all UART interrupts by writing to the Interrupt Clear Register (ICR).
    mmioWrite(Registers.UART0_ICR, 0xFFFFFFFF);

    // Set integer part of the baud rate divisor to 1 in the Integer Baud Rate Divisor Register (IBRD).
    // Baud rate = (Frequency) / (16 * (IBRD + FBRD / 64))
    mmioWrite(Registers.UART0_IBRD, 1);

    // Set fractional part of the baud rate divisor to 40 in the Fractional Baud Rate Divisor Register (FBRD).
    mmioWrite(Registers.UART0_FBRD, 40);

    // Set line control options in the Line Control Register (LCRH):
    // - Enable 8-bit word length (bits [5:6] = 1).
    // - Enable FIFOs (bit [4] = 1).
    mmioWrite(Registers.UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));

    // Enable UART interrupts by setting bits in the Interrupt Mask Set/Clear Register (IMSC):
    // - Bits 1, 4, 5, 6, 7, 8, 9, and 10 to enable various interrupt sources (e.g., RX, TX).
    mmioWrite(Registers.UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) |
                                     (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));

    // Re-enable UART0 by setting the Control Register (CR):
    // - Bit 0 enables the UART.
    // - Bit 8 enables transmit.
    // - Bit 9 enables receive.
    mmioWrite(Registers.UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
}

