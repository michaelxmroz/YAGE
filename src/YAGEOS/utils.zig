
pub fn delay(cycles : i32) void
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

pub fn writeVectorTableBaseAddr(addr:usize) void
{
        asm volatile(
         \\ writeVectorTableBaseAddr:
         \\ msr vbar_el1, x0
         \\ ret
         :
		 : [ad] "x0"(addr)
         );
}


//Don't trap the SIMD and FP operations in EL[0,1]
pub fn enableSIMDAndFPOps() void
{
        asm volatile(
         \\ mrs    x1, cpacr_el1
         \\ mov    x0, #(3 << 20)
         \\ orr    x0, x1, x0
         \\ msr    cpacr_el1, x0
         :
         :
        );
}

pub fn writeSCTLR_EL1(value:u64) void
{
    asm volatile
    (
    \\ writeSCTLR:
    \\ mov x0 , #0x1
    \\ tlbi vmalle1                // Invalidate the entire TLB
    \\ dsb sy                      // Ensure all memory transactions complete
    \\ isb                         // Synchronize context
    \\ msr SCTLR_EL1, x0
    \\ orr x0, x0, x0
    \\ nop
    \\ nop
    \\ orr x0, x0, x0
    \\ ret
    :
    : [ad] "x0"(value)
    );
}

pub fn invalidateTLB() void
{
    asm volatile (
        \\ tlbi vmalle1                // Invalidate the entire TLB
        \\ dsb sy                      // Ensure all memory transactions complete
        \\ isb                         // Synchronize context
    );
}

pub fn getEL() usize
{
       return asm volatile(
        \\ get_el:
        \\ mrs x0, CurrentEL
        \\ lsr x0, x0, #2
        \\ ret
         : [ret] "={x0}" (-> usize),
		 : 
         );

}

pub fn hang() noreturn
{
    while (true) {}
}