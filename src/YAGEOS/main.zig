
const mmio = @import("mmio.zig");

const raspi = 4;

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
    \\    ldr     x5, =__stack_top
    \\    mov     sp, x5 
    \\    ldr     x5, =__bss_start
    \\    ldr     x7, =__bss_end
    \\    ldr     w6, =__bss_size
    \\ 1: cbz     w6, 2f
    \\    str     xzr, [x5], #8
    \\    sub     w6, w6, #1
    \\    cbnz    w6, 1b
    \\ 
    \\ 2: bl      main
    :
    :
    );

    while (true) {}
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

const MBOX_REGS = enum (u32) 
{
    const VIDEOCORE_MBOX = (mmio.MMIO_BASE + 0x0000B880);

    MBOX_READ      = (VIDEOCORE_MBOX + 0x0),
    MBOX_POLL      = (VIDEOCORE_MBOX + 0x10),
    MBOX_SENDER    = (VIDEOCORE_MBOX + 0x14),
    MBOX_STATUS    = (VIDEOCORE_MBOX + 0x18),
    MBOX_CONFIG    = (VIDEOCORE_MBOX + 0x1C),
    MBOX_WRITE     = (VIDEOCORE_MBOX + 0x20),
    MBOX_RESPONSE  = 0x80000000,
    MBOX_FULL      = 0x80000000,
    MBOX_EMPTY     = 0x40000000
};

var mbox: [36]u32 align(16) = undefined;

export fn main() void
{
    mmio.uartInit();
    mmio.uartSendString("Hello, Raspberry Pi 4!\n");
}
