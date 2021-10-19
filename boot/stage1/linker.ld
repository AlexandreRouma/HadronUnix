ENTRY(stage1_start)

SECTIONS {
	. = 0x7C00;

    .boot BLOCK(512) : ALIGN(512) {
		*(.boot)
	}
}