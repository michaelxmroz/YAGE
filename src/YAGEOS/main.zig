const mmio = @import("mmio.zig");
const renderer = @import("renderer.zig");
const log = @import("log.zig");
const std = @import("std");
const utils = @import("utils.zig");
const defs = @import("defs.zig");
const mmu = @import("mmu.zig");

fn dummyInterruptHandler(exceptionType: u32, execptionType2: u32) noreturn 
{
    _ = exceptionType;
    _ = execptionType2;
    mmio.uartSendString("\n!UNIMPLEMENTED INTERRUPT HANDLER!\n");
    utils.hang();
}

pub fn setupInterruptVectorTable() void 
{
    asm volatile (
        \\ setupInterruptVectorTable:
        // \\ movz x1, #0xD61F
        \\ movk x1, #0xD61F, lsl #16
        \\ mov x8, #0
        \\ loop:
        \\ cmp x8, #16
        \\ b.ge end
        \\ lsl x4, x8, #7
        \\ add x2, x5, x4
        \\ str x6, [x2]
        \\ add x2, x2, #8
        \\ str x1, [x2]
        \\ add x8, x8, #1
        \\ b loop
        \\ end:
        \\ ret
        : // output operands
        : [buf] "x5" (@intFromPtr(&defs.VECTOR_TABLE)),
          [jumpAddr] "x6" (&dummyInterruptHandler),
          [count] "x7" (16),
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8" // clobbered registers
    );
}

export fn initMMU() void 
{
    mmu.initMMU();
}

export fn _start() callconv(.Naked) noreturn 
{
    const asm_str =
        \\    mrs    x0, mpidr_el1        
        \\    and    x0, x0,#0xFF        // Check processor id
        \\    cbz    x0, setupCPU        // Hang for all non-primary CPU
        \\    b    proc_hang
        \\
        \\ proc_hang: 
        \\    b proc_hang
        \\
        \\ setupCPU:
        \\    ldr x0, ={0}
        \\    msr sctlr_el1, x0	
        \\    ldr x0, ={1}
        \\    msr hcr_el2, x0
        \\    ldr x0, ={2}
        \\    msr scr_el3, x0
        \\    ldr x0, ={3}
        \\    msr spsr_el3, x0
        \\    ldr x0, ={4}
        \\    msr cpacr_el1, x0
        \\    ldr x0, ={5}
        \\    msr tcr_el1, x0
        \\    ldr	x0, ={6}
        \\    msr	mair_el1, x0
        \\    adr x0, setupMMU		
        \\    msr elr_el3, x0
        \\    eret	
        \\ setupMMU:
        \\ mov sp, #{7}
        \\ adr	x0, __bss_start
        \\ adr	x1, __bss_end
        \\ sub	x1, x1, x0
        \\ bl 	memzero
        \\ 
        \\ bl 	initMMU
        \\ adrp x0, pg_dir	
        \\ msr	ttbr0_el1, x0
        \\ msr	ttbr1_el1, x0
        \\ mrs x0, sctlr_el1
        \\ mov x1, #{8}
        \\ orr x0, x0, x1
        \\ msr SCTLR_EL1, x0
        \\ orr x0, x0, x0
        \\ nop
        \\ nop
        \\ orr x0, x0, x0
        \\ bl main
        \\ .globl id_pgd_addr
        \\ id_pgd_addr:
        \\ adrp x0, pg_dir
        \\ ret
        \\.globl memzero
        \\memzero:
        \\    str xzr, [x0], #8
        \\    subs x1, x1, #8
        \\   b.gt memzero
        \\    ret
    ;

    //const asm_str2 = "mov x30, {}";
    const formated_asm = std.fmt.comptimePrint(asm_str, .{ defs.SCTLR_VALUE_MMU_DISABLED, defs.HCR_VALUE, defs.SCR_VALUE, defs.SPSR_VALUE, defs.CPACR_VALUE, defs.TCR_VALUE, defs.MAIR_VALUE, defs.LOW_MEMORY, defs.SCTLR_VALUE_MMU_ENABLED });
    asm volatile (formated_asm);

    while (true) {}
}

pub fn panic(message: []const u8, stack_trace: ?*std.builtin.StackTrace, _: ?usize) noreturn 
{
    mmio.uartSendString("\n!KERNEL PANIC!\n");
    mmio.uartSendString(message);
    mmio.uartSendString("\n");
    _ = stack_trace;
    utils.hang();
}

export fn main() void 
{
    setupInterruptVectorTable();
    utils.writeVectorTableBaseAddr(@intFromPtr(&defs.VECTOR_TABLE));

    mmio.uartInit();
    mmio.uartSendString("Success\n");

    log.INFO("Hello, Raspberry Pi 4!\n", .{});
    //renderer.initFramebuffer();

    utils.hang();
}
