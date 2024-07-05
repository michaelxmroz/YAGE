// Zig Raspi4 Kernel, adapted from https://wiki.osdev.org/Raspberry_Pi_Bare_Bones

const std = @import("std");

const MMIO_BASE: u32 = 0xFE000000;
const raspi = 4;

export var stack_bytes: [16 * 1024]u8 align(16) linksection(".bss") = undefined;
const stack_bytes_slice = stack_bytes[0..];

export fn _start() callconv(.Naked) noreturn 
{
    asm volatile (
    \\    mrs    x0, mpidr_el1        
    \\    and    x0, x0,#0xFF        // Check processor id
    \\    cbz    x0, setupMain        // Hang for all non-primary CPU
    \\    b    proc_hang
    \\
    \\ proc_hang: 
    \\    b proc_hang
    \\ setupMain:
    \\    mov     sp, x5 
    \\    ldr     x5, =__bss_start
    \\    ldr     w6, =__bss_size
    \\ 1: cbz     w6, 2f
    \\    str     xzr, [x5], #8
    \\    sub     w6, w6, #1
    \\    cbnz    w6, 1b
    \\ 
    \\ 2: bl      main
    :
    : [stk] "x5" (@intFromPtr(&stack_bytes_slice) + @sizeOf(@TypeOf(stack_bytes_slice))),
    );

    while (true) {}
}

fn mmioWrite(reg : Registers, data : u32) void
{
    const ptr: *volatile u32 = @ptrFromInt(MMIO_BASE + @intFromEnum(reg));
    ptr.* = data;
}

fn mmioRead(reg : Registers) u32
{
    const ptr: *volatile u32 = @ptrFromInt(MMIO_BASE + @intFromEnum(reg));
    return ptr.*;
}

fn delay(cycles : i32) void
{
    	asm volatile(
         \\ delay:
         \\ subs x0, x0, #1
         \\ bne delay
         \\ ret
         :
		 : [count] "x0"(cycles)
         );
}

const Registers = enum (u32)
{
    // The offsets for reach register.
    const GPIO_BASE = 0x200000;
    // The base address for UART.
    const UART0_BASE = (GPIO_BASE + 0x1000);
    const MBOX_BASE    = 0xB880;
 
    // Controls actuation of pull up/down to ALL GPIO pins.
    GPPUD = (GPIO_BASE + 0x94),
 
    // Controls actuation of pull up/down for specific GPIO pin.
    GPPUDCLK0 = (GPIO_BASE + 0x98),

    // The offsets for reach register for the UART.
    UART0_DR     = (UART0_BASE + 0x00),
    UART0_RSRECR = (UART0_BASE + 0x04),
    UART0_FR     = (UART0_BASE + 0x18),
    UART0_ILPR   = (UART0_BASE + 0x20),
    UART0_IBRD   = (UART0_BASE + 0x24),
    UART0_FBRD   = (UART0_BASE + 0x28),
    UART0_LCRH   = (UART0_BASE + 0x2C),
    UART0_CR     = (UART0_BASE + 0x30),
    UART0_IFLS   = (UART0_BASE + 0x34),
    UART0_IMSC   = (UART0_BASE + 0x38),
    UART0_RIS    = (UART0_BASE + 0x3C),
    UART0_MIS    = (UART0_BASE + 0x40),
    UART0_ICR    = (UART0_BASE + 0x44),
    UART0_DMACR  = (UART0_BASE + 0x48),
    UART0_ITCR   = (UART0_BASE + 0x80),
    UART0_ITIP   = (UART0_BASE + 0x84),
    UART0_ITOP   = (UART0_BASE + 0x88),
    UART0_TDR    = (UART0_BASE + 0x8C),
 
    // The offsets for Mailbox registers

    MBOX_READ    = (MBOX_BASE + 0x00),
    MBOX_STATUS  = (MBOX_BASE + 0x18),
    MBOX_WRITE   = (MBOX_BASE + 0x20)

};

// A Mailbox message with set clock rate of PL011 to 3MHz tag
//const mbox :  *align(16) volatile u32 = u32[9]{
 //   9*4, 0, 0x38002, 12, 8, 2, 3000000, 0 , 0
//};

const mbox align(16) = [_] u32 {
    9*4, 0, 0x38002, 12, 8, 2, 3000000, 0 , 0
};

fn uartInit() void
{
	// Disable UART0.
	mmioWrite(Registers.UART0_CR, 0x00000000);
	// Setup the GPIO pin 14 && 15.
 
	// Disable pull up/down for all GPIO pins & delay for 150 cycles.
	mmioWrite(Registers.GPPUD, 0x00000000);
	delay(150);
 
	// Disable pull up/down for pin 14,15 & delay for 150 cycles.
	mmioWrite(Registers.GPPUDCLK0, (1 << 14) | (1 << 15));
	delay(150);
 
	// Write 0 to GPPUDCLK0 to make it take effect.
	mmioWrite(Registers.GPPUDCLK0, 0x00000000);
 
	// Clear pending interrupts.
	mmioWrite(Registers.UART0_ICR, 0x7FF);
 
	// Set integer & fractional part of baud rate.
	// Divider = UART_CLOCK/(16 * Baud)
	// Fraction part register = (Fractional part * 64) + 0.5
	// Baud = 115200.
 
	// For Raspi3 and 4 the UART_CLOCK is system-clock dependent by default.
	// Set it to 3Mhz so that we can consistently set the baud rate
	if (raspi >= 3) {
		// UART_CLOCK = 30000000;
        const fullSet : usize = 0xF;
		const r : u32 = @intCast((@intFromPtr(&mbox) & ~fullSet) | 8);
		// wait until we can talk to the VC
		while ( mmioRead(Registers.MBOX_STATUS) & 0x80000000 > 0 ) { }
		// send our message to property channel and wait for the response
		mmioWrite(Registers.MBOX_WRITE, r);
		while ( ((mmioRead(Registers.MBOX_STATUS) & 0x40000000) > 0) or (mmioRead(Registers.MBOX_READ) != r) ) { }
	}
 
	// Divider = 3000000 / (16 * 115200) = 1.627 = ~1.
	mmioWrite(Registers.UART0_IBRD, 1);
	// Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
	mmioWrite(Registers.UART0_FBRD, 40);
 
	// Enable FIFO & 8 bit data transmission (1 stop bit, no parity).
	mmioWrite(Registers.UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));
 
	// Mask all interrupts.
	mmioWrite(Registers.UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) |
	                       (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));
 
	// Enable UART0, receive & transfer part of UART.
	mmioWrite(Registers.UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
}

//export fn main(dtb_ptr32 : u64, x1 : u64, x2 : u64, x3 : u64) void
export fn main() void
{
    uartInit();
    //const emu = emulator_header.CreateEmulatorHandle();

	//emu.SetLoggerCallback(LogMessage);

    //defer emulator_header.Delete(emu);

    //print("hello, world\n",.{});
    //    print("hello, world\n",.{});
}
