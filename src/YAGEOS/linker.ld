ENTRY(_start)

SECTIONS
{
    .text.boot : { *(.text.boot) }
	.text : { *(.text) }
	.data : { *(.data) }

	. = ALIGN(0x8);
	__bss_start = .;
	.bss : { *(.bss*) }
	__bss_end = .;
	. = ALIGN(0x00001000);
	pg_dir = .;
	.data.pgd : { . += (6 * (1 << 12)); }
    .rodata : 
	{ 
       *(.rodata)
        __debug_info_start = .;
        KEEP(*(.debug_info))
        __debug_info_end = .;
        __debug_abbrev_start = .;
        KEEP(*(.debug_abbrev))
        __debug_abbrev_end = .;
        __debug_str_start = .;
        KEEP(*(.debug_str))
        __debug_str_end = .;
        __debug_line_start = .;
        KEEP(*(.debug_line))
        __debug_line_end = .;
        __debug_ranges_start = .;
        KEEP(*(.debug_ranges))
        __debug_ranges_end = .;
	}
}