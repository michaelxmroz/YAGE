const std = @import("std");

// In Zig 0.13 the compiler reeealy wants to inline this function, resulting in duplicate symbol errors from LLVM. Thanks, Zig.
pub fn delay(cycles : i32) void
{
    	asm volatile(
         \\ delayme:
         \\     subs x0, x0, #1
         \\     bne delayme
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

pub fn enableIRQ() void
{
    asm volatile(
         \\ enableIRQ:
         \\ msr daifclr, #2
         \\ ret
         :
		 :
         );
}

pub fn disableIRQ() void
{
    asm volatile(
        \\ enableIRQ:
        \\ msr daifset, #2
        \\ ret
        :
		:
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