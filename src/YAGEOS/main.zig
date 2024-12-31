const mmio = @import("mmio.zig");
const renderer = @import("renderer.zig");
const log = @import("log.zig");
const std = @import("std");
const utils = @import("utils.zig");
const defs = @import("defs.zig");
const mmu = @import("mmu.zig");

const cpp = @cImport({
    @cInclude("Emulator_C.h");
});

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
        :
        : [buf] "x5" (@intFromPtr(&defs.VECTOR_TABLE)),
          [jumpAddr] "x6" (&dummyInterruptHandler),
          [count] "x7" (16),
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8"
    );
}

export fn initMMU() void 
{
    mmu.initMMU();
}

export fn _start() callconv(.Naked) noreturn 
{
    // Kernel entry point
    // Set up the CPU and initialize the MMU
    //
    const asm_str =
        \\    mrs    x0, mpidr_el1        
        \\    and    x0, x0, #0xFF        // Check processor ID (lower 8 bits)
        \\    cbz    x0, setupCPU         // If ID is zero (primary CPU), continue to setupCPU
        \\    b      proc_hang            // Else, loop indefinitely to halt non-primary CPUs
        \\
        \\ proc_hang: 
        \\    b proc_hang                 // Hang by branching to itself indefinitely
        \\
        \\ setupCPU:
        \\    ldr x0, ={0}
        \\    msr sctlr_el1, x0           // Set System Control Register (SCTLR) to disable MMU, I-cache, and D-cache
        \\
        \\    ldr x0, ={1}
        \\    msr hcr_el2, x0             // Set HCR_EL2 to enter non-secure state, 64-bit EL1/0 execution
        \\
        \\    ldr x0, ={2}
        \\    msr scr_el3, x0             // Set SCR_EL3 to allow entry to EL2/EL1 in 64-bit non-secure mode
        \\
        \\    ldr x0, ={3}
        \\    msr spsr_el3, x0            // Set SPSR_EL3 to mask all interrupts and return to EL1h
        \\
        \\    ldr x0, ={4}
        \\    msr cpacr_el1, x0           // Set CPACR_EL1 to enable access to SIMD/FPU (don't trap SIMD/FP registers)
        \\
        \\    ldr x0, ={5}
        \\    msr tcr_el1, x0             // Set TCR_EL1 to configure translation regime for virtual memory in EL1
        \\
        \\    ldr x0, ={6}
        \\    msr mair_el1, x0            // Set MAIR_EL1 to define memory attributes for normal and device memory
        \\
        \\    adr x0, setupMMU
        \\    msr elr_el3, x0             // Set Exception Link Register EL3 to start executing at setupMMU
        \\    eret                        // Exception return to EL1, transferring control to setupMMU
        \\
        \\ setupMMU:
        \\    mov sp, #{7}                // Set stack pointer to LOW_MEMORY (initial stack for kernel)
        \\
        \\    adr x0, __bss_start
        \\    adr x1, __bss_end
        \\    sub x1, x1, x0
        \\    bl  memzero                 // Call memzero to clear the .bss section (zero-initialize)
        \\
        \\    bl  initMMU                 // Initialize the MMU, setting up the page tables and memory mappings
        \\
        \\    adrp x0, pg_dir             // Load base address of the page directory into x0
        \\    msr ttbr0_el1, x0           // Set TTBR0_EL1 to page directory for user/kernel translation table base
        \\    msr ttbr1_el1, x0           // Set TTBR1_EL1 to page directory for higher memory regions
        \\
        \\    mrs x0, sctlr_el1           // Read current SCTLR_EL1 configuration
        \\    mov x1, #{8}
        \\    orr x0, x0, x1              // Enable MMU and caches in SCTLR_EL1
        \\    msr sctlr_el1, x0           // Write updated SCTLR_EL1 with MMU enabled
        \\    bl main                     // Call main function to start the kernel
        \\
        \\ .globl id_pgd_addr
        \\ id_pgd_addr: 
        \\    adrp x0, pg_dir             // Load page directory base address
        \\    ret
        \\
        \\ .globl memzero
        \\ memzero:
        \\    str xzr, [x0], #8           // Write zero to memory in 8-byte chunks
        \\    subs x1, x1, #8             // Decrease remaining size by 8
        \\    b.gt memzero                // Loop until entire range is cleared
        \\    ret
    ;

    const formated_asm = std.fmt.comptimePrint(
        asm_str, .{ 
            defs.SCTLR_VALUE_MMU_DISABLED,
            defs.HCR_VALUE,               
            defs.SCR_VALUE,               
            defs.SPSR_VALUE,              
            defs.CPACR_VALUE,             
            defs.TCR_VALUE,               
            defs.MAIR_VALUE,              
            defs.LOW_MEMORY,              
            defs.SCTLR_VALUE_MMU_ENABLED  
        }
    );

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

    utils.enableIRQ();

    mmio.uartInit();
    mmio.uartSendString("Success\n");

    const prevIRQVal = mmio.mmioReadDirect( mmio.IRQ_GPU_ENABLE2);
    mmio.mmioWriteDirect( mmio.IRQ_GPU_ENABLE2,( prevIRQVal & ~@as(u32,mmio.IRQ_GPU_FAKE_ISR)) | mmio.IRQ_GPU_FAKE_ISR );

    log.INFO("Hello, Raspberry Pi {}!\n", .{defs.raspi});
    renderer.initFramebuffer();

    renderer.drawRect(150,150,400,400,renderer.Color{.components = renderer.Components{.a = 0xFF, .r = 0xFF, .g = 0x0, .b = 0x0 }});

    const emu = cpp.CreateEmulatorHandle();
    cpp.Delete(emu);

    utils.hang();
}
