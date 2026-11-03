# Handwritten assembly reference:
# https://github.com/hansbonini/psx_tomba/blob/b0aab7ef43785b257f66fb007df36b222e1a3665/src/psyq/libapi/l02.s
.include "syslib/bios.inc"

.section .text, "ax"

BIOS_CALL SysEnqIntRP, 0xC0, 0x2
