ENTRY(_start)

SECTIONS
{
    .text.boot : { *(.text.boot) }
	.text : { *(.text) }
	.rodata : { *(.rodata) }
	.data : { *(.data) }
	. = ALIGN(0x8);
	__bss_start = .;
	.bss : { *(.bss*) }
	__bss_end = .;
	. = ALIGN(0x00001000);
	pg_dir = .;
	.data.pgd : { . += (6 * (1 << 12)); }
}