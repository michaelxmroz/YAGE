
const mmio = @import("mmio.zig");
const renderer = @import("renderer.zig");
const log = @import("log.zig");
const std = @import("std");
const utils = @import("utils.zig");

const raspi = 4;

const SCTLR_RESERVED             =     (3 << 28) | (3 << 22) | (1 << 20) | (1 << 11);
const SCTLR_EE_LITTLE_ENDIAN     =     (0 << 25);
const SCTLR_EOE_LITTLE_ENDIAN    =     (0 << 24);
const SCTLR_I_CACHE_DISABLED     =     (0 << 12);
const SCTLR_D_CACHE_DISABLED     =     (0 << 2);
const SCTLR_I_CACHE_ENABLED      =     (1 << 12);
const SCTLR_D_CACHE_ENABLED      =     (1 << 2);
const SCTLR_MMU_DISABLED         =     (0 << 0);
const SCTLR_MMU_ENABLED          =     (1 << 0);

const SCTLR_VALUE_MMU_DISABLED =	(SCTLR_RESERVED | SCTLR_EE_LITTLE_ENDIAN | SCTLR_I_CACHE_DISABLED | SCTLR_D_CACHE_DISABLED | SCTLR_MMU_DISABLED);
const SCTLR_VALUE_MMU_ENABLED =	    (SCTLR_RESERVED | SCTLR_EE_LITTLE_ENDIAN | SCTLR_I_CACHE_ENABLED | SCTLR_D_CACHE_ENABLED | SCTLR_MMU_ENABLED);

// ***************************************
// HCR_EL2, Hypervisor Configuration Register (EL2), Page 2487 of AArch64-Reference-Manual.
// ***************************************

const HCR_RW	=    			(1 << 31);
const HCR_VALUE	=		HCR_RW;

// ***************************************
// SCR_EL3, Secure Configuration Register (EL3), Page 2648 of AArch64-Reference-Manual.
// ***************************************

const SCR_RESERVED	=   		(3 << 4);
const SCR_RW		=		(1 << 10);
const SCR_NS		=		(1 << 0);
const SCR_VALUE	    =	    	(SCR_RESERVED | SCR_RW | SCR_NS);

// ***************************************
// SPSR_EL3, Saved Program Status Register (EL3) Page 389 of AArch64-Reference-Manual.
// ***************************************

const SPSR_MASK_ALL =			(7 << 6);
const SPSR_EL1h		=	(5 << 0);
const SPSR_VALUE	=		(SPSR_EL1h);


const VECTOR_TABLE: [16]usize align(128) = undefined;

const PAGE_TABLE_SIZE = 512; 
const PAGE_TABLE_ALIGNMENT = 0x4000;  

const L1_RAM_ATTRS = 0x0000000000000045;
const L1_DEVICE_ATTRS = 0x0000000000000041;

const L0_ATTR = 0xC3;

var L0_PAGE_TABLE: [PAGE_TABLE_SIZE]usize align(PAGE_TABLE_ALIGNMENT) = undefined;
var L1_PAGE_TABLE: [PAGE_TABLE_SIZE]usize align(PAGE_TABLE_ALIGNMENT) = undefined;

extern var __page_table : *u64;

fn dummyInterruptHandler(exceptionType:u32, execptionType2:u32) noreturn 
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
        : [buf] "x5" (@intFromPtr(&VECTOR_TABLE)), [jumpAddr] "x6" (&dummyInterruptHandler), [count] "x7" (16)
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8" // clobbered registers
    );
}

fn setTableEntry(tableAddr: []usize, index: usize, phys_addr: usize, attrs: usize) void 
{
    // Populate the entry with the physical address and attributes
    tableAddr[index] = (phys_addr & 0xFFFF_FFFF_F000) | attrs;
}

fn setMemoryAttributeIndirectionEntries() void
{
    asm volatile
    (
    \\ setMemoryAttributeIndirectionEntries:
    \\  mov x0, 0x4444               // 0xFF for Normal Cacheable, 0x44 for Device Non-cacheable
    \\  msr MAIR_EL1, x0
    \\ ret
    :
    :
    );
}

fn setPageTableBaseAddr(addr : *usize) void
{
    asm volatile
    (
    \\ setPageTableBaseAddr:
    \\ mov x0, x8
    \\ ldr x22, [x0]
    \\ msr TTBR0_EL1, x0           // Set TTBR0_EL1 to point to the page table
    \\ ret
    :
    : [page_table] "x8" (addr)
    );
}

const TCR_VALUE = 0x80100010;

fn hardcodeTableEntries(t0 : *usize, t1 : *usize) void
{
    asm volatile
    (
    \\ hardcodeTableEntries:
    \\ mov x0, #0
    \\ DC IVAC, x0   //Invalidate entire data cache (all levels)
    \\ nop
    \\ mov x8, %[page_table0]
    \\ mov x9, %[page_table1]
    \\ mov x0, #0xE3
    \\ str x0, [x9]
    \\ ldr x23, [x9] 
    \\ mov x0, #0x1
    \\ orr x9, x9, x0
    \\ str x9, [x8]
    \\ ldr x22, [x8] 
    \\
    \\  mov x0, 0x4444               // 0xFF for Normal Cacheable, 0x44 for Device Non-cacheable
    \\  msr MAIR_EL1, x0
    \\
    \\ tlbi vmalle1                // Invalidate the entire TLB
    \\ dsb sy                      // Ensure all memory transactions complete
    \\ isb                         // Synchronize context
    \\
    \\ msr TTBR0_EL1, x8           // Set TTBR0_EL1 to point to the page table
    \\ msr TTBR1_EL1, x8           // Set TTBR0_EL1 to point to the page table
    \\ nop
    \\ mov x0, %[tcr_val]
    \\ msr tcr_el1, x0
    \\ nop
    \\ isb                         // Synchronize context
    \\ mov x0 , #0x1
    \\ msr SCTLR_EL1, x0
    \\ isb                         // Synchronize context
    \\ orr x0, x0, x0
    \\ nop
    \\ nop
    \\ orr x0, x0, x0
    \\ ret
    :
    : [page_table0] "r" (t0), [page_table1] "r" (t1) , [tcr_val] "r" (TCR_VALUE)
    : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9" // clobbered registers
    );
}

fn initPageTable() void 
{
    //setTableEntry(&L0_PAGE_TABLE, 0, @intFromPtr(&L1_PAGE_TABLE[0]), L0_ATTR);
   // setTableEntry(&L1_PAGE_TABLE, 0, 0x0, L1_RAM_ATTRS);
    setMemoryAttributeIndirectionEntries();
    hardcodeTableEntries(&L0_PAGE_TABLE[0], &L1_PAGE_TABLE[0]);
    // Map first 1 GB of RAM to 1 GB virtual address space
    //setL1Entry(0, 0x0000000, L1_RAM_ATTRS);

    // Map device memory
    //setL1Entry(1, mmio.MMIO_BASE, L1_DEVICE_ATTRS);
    
    //__page_table.* = 0x45;
    //setHardcodedPageTable();

    //const tablePtr : *u64 = &L0_PAGE_TABLE[0];

    //setPageTableBaseAddr(tablePtr);

    utils.writeSCTLR_EL1(SCTLR_MMU_ENABLED);
    utils.invalidateTLB();
}

export fn _start() callconv(.Naked) noreturn 
{
    asm volatile 
    (
    \\    mrs    x0, mpidr_el1        
    \\    and    x0, x0,#0xFF        // Check processor id
    \\    cbz    x0, setupCPU        // Hang for all non-primary CPU
    \\    b    proc_hang
    \\
    \\ proc_hang: 
    \\    b proc_hang
    \\
    \\ setupCPU:
	\\    msr sctlr_el1, %[sctlr]	
    \\
    \\    //Bullshit MMU setup
    \\
    \\ mov x0, #0
    \\ DC IVAC, x0   //Invalidate entire data cache (all levels)
    \\ nop
    \\ mov x8, %[page_table0]
    \\ mov x9, %[page_table1]
    \\ mov x0, #0xE3
    \\ str x0, [x9]
    \\ ldr x23, [x9] 
    \\ mov x0, #0x1
    \\ orr x9, x9, x0
    \\ str x9, [x8]
    \\ ldr x22, [x8] 
    \\
    \\  mov x0, 0x4444               // 0xFF for Normal Cacheable, 0x44 for Device Non-cacheable
    \\  msr MAIR_EL1, x0
    \\
    \\ tlbi vmalle1                // Invalidate the entire TLB
    \\ dsb sy                      // Ensure all memory transactions complete
    \\ isb                         // Synchronize context
    \\
    \\ msr TTBR0_EL1, x8           // Set TTBR0_EL1 to point to the page table
    \\ msr TTBR1_EL1, x8           // Set TTBR0_EL1 to point to the page table
    \\ nop
    \\ mov x0, %[tcr_val]
    \\ msr tcr_el1, x0
    \\ nop
    \\ isb                         // Synchronize context
    \\ mov x0 , #0x1
    \\ msr SCTLR_EL1, x0
    \\ isb                         // Synchronize context
    \\ orr x0, x0, x0
    \\ nop
    \\ nop
    \\ orr x0, x0, x0
    \\
    \\ //Bullshit MMU setup end
    \\
	\\    msr hcr_el2, %[hcr]
	\\    msr scr_el3, %[scr]
	\\    msr spsr_el3, %[spsr]
	\\    adr x0, setupMain		
	\\    msr elr_el3, x0
	\\    eret	
    \\
    \\ setupMain:
    \\    mov     sp, #4194304 
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
    : [sctlr] "r" (SCTLR_VALUE_MMU_DISABLED), [hcr] "r" (HCR_VALUE), [scr] "r" (SCR_VALUE), [spsr] "r" (SPSR_VALUE), [page_table0] "r" ((&L0_PAGE_TABLE[0])), [page_table1] "r" ((&L1_PAGE_TABLE[0])) , [tcr_val] "r" (TCR_VALUE)
    );

    while (true) {}
}

//pub fn panic(message: []const u8, stack_trace: ?*std.builtin.StackTrace, _: ?usize) noreturn 
//{
//    mmio.uartSendString("\n!KERNEL PANIC!\n");    
//    mmio.uartSendString(message);   
//    mmio.uartSendString("\n");
//    _ = stack_trace;
//    utils.hang();
//}

export fn main() void
{
    //initPageTable();
    utils.enableSIMDAndFPOps();
    utils.hang();
    setupInterruptVectorTable();
    utils.writeVectorTableBaseAddr(@intFromPtr(&VECTOR_TABLE));
    utils.hang();
    mmio.uartInit();
    mmio.uartSendString("Success\n");

    const el = utils.getEL();
    switch (el) {
        0=>mmio.uartSendString("Execution Level 0\n"),
        1=>mmio.uartSendString("Execution Level 1\n"),
        2=>mmio.uartSendString("Execution Level 2\n"),
        3=>mmio.uartSendString("Execution Level 3\n"),
        else => mmio.uartSendString("Unknown Execution Level \n")
    }
    
    log.INFO("Hello, Raspberry Pi 4!\n", .{});
    //renderer.initFramebuffer();
}
