/* The bootloader will look at this image and start execution at the symbol
   designated as the entry point. */
ENTRY(stage2_start)

SECTIONS
{
	/* Begin putting sections at 1 MiB, a conventional place for kernels to be
	   loaded at by the bootloader. */
	. = 0x7E00;

	/* First put the multiboot header, as it is required to be put very early
	   early in the image or the bootloader won't recognize the file format.
	   Next we'll put the .text section. */

    .header BLOCK(16) : ALIGN(16)
	{
		*(.header)
		*(.realmode_caller)
	}

	.text BLOCK(16) : ALIGN(16)
	{
		*(.text)
	}

	/* Read-only data. */
	.rodata BLOCK(16) : ALIGN(16)
	{
		*(.rodata)
	}

	/* Read-write data (initialized) */
	.data BLOCK(16) : ALIGN(16)
	{
		*(.data)
	}

	/* Read-write data (uninitialized) and stack */
	.bss BLOCK(16) : ALIGN(16)
	{
		*(COMMON)
		*(.bss)
	}

	/* Space after the loader */
	.space_after_loader BLOCK(16) : ALIGN(16)
	{
		*(.space_after_loader)
	}
}