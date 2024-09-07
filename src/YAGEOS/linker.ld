ENTRY(_start)

SECTIONS
{
    . = 0x80000;
    .text.boot : ALIGN(8) { *(.text.boot) }
	.text : ALIGN(8) { *(.text) }
	.rodata : ALIGN(8) { *(.rodata) }
	.data : ALIGN(8) { *(.data) }
	. = ALIGN(0x8);
	__bss_start = .;
	.bss : { *(.bss*) }
	__page_table = ALIGN(0x4000);
	__bss_end = .;
    __bss_size = __bss_end - __bss_start;
}

ASSERT(__bss_end <= 0x900000, "Memory overflow: sections exceed available RAM");